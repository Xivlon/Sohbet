#!/usr/bin/env python3
"""
SQLite users CLI (updated to include additional categories)

Usage examples:
  python3 sqlite_users.py --db ./app.db init
  python3 sqlite_users.py --db ./app.db add --username alice --email alice@example.com --name "Alice Smith" --id-no "ID1234" --clearance "Top Secret" --institution "Example Uni" --specialization "CS" --grad-year 2020 --phone "+1-555-1234" --role admin
  python3 sqlite_users.py --db ./app.db list
  python3 sqlite_users.py --db ./app.db auth --username alice
  python3 sqlite_users.py --db ./app.db passwd --username alice

Dependencies:
  pip install bcrypt
"""

import argparse
import sqlite3
import bcrypt
import getpass
import re
from pathlib import Path
import sys
from datetime import datetime

DEFAULT_DB = "app.db"
USERNAME_RE = re.compile(r"^[A-Za-z0-9_.-]{3,150}$")
EMAIL_RE = re.compile(r"^[^@]+@[^@]+\.[^@]+$")
PHONE_RE = re.compile(r"^[\d+\-\s()]{7,30}$")  # permissive phone check


def connect(db_path: Path):
    conn = sqlite3.connect(str(db_path))
    conn.row_factory = sqlite3.Row
    # recommended pragmas
    conn.execute("PRAGMA foreign_keys = ON;")
    conn.execute("PRAGMA journal_mode = WAL;")
    return conn


CREATE_TABLE_SQL = """
CREATE TABLE IF NOT EXISTS users (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  id_no TEXT UNIQUE,                         -- "ID No"
  username TEXT NOT NULL UNIQUE,
  name TEXT,                                  -- Full name
  email TEXT NOT NULL UNIQUE,
  password TEXT NOT NULL,
  clearance_level TEXT,                       -- Clearance level
  institution TEXT,
  specialization TEXT,
  graduation_year INTEGER,
  phone TEXT,
  role TEXT DEFAULT 'user',
  account_warnings INTEGER DEFAULT 0,
  created_at TEXT NOT NULL DEFAULT (datetime('now'))
);
CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);
CREATE INDEX IF NOT EXISTS idx_users_id_no ON users(id_no);
"""

def init_db(db_path: Path):
    db_path.parent.mkdir(parents=True, exist_ok=True)
    conn = connect(db_path)
    cur = conn.cursor()
    cur.executescript(CREATE_TABLE_SQL)
    conn.commit()
    conn.close()
    print(f"Initialized database at {db_path.resolve()}")


def validate_username(username: str):
    if not USERNAME_RE.match(username):
        raise ValueError("username must be 3-150 chars, letters/numbers/._- allowed")
    return username


def validate_email(email: str):
    if not EMAIL_RE.match(email):
        raise ValueError("invalid email address")
    return email


def validate_graduation_year(year):
    if year is None:
        return None
    try:
        y = int(year)
    except Exception:
        raise ValueError("graduation year must be an integer")
    now_year = datetime.utcnow().year
    if y < 1900 or y > now_year + 10:
        raise ValueError(f"graduation year must be between 1900 and {now_year + 10}")
    return y


def validate_phone(phone):
    if phone is None or phone == "":
        return None
    if not PHONE_RE.match(phone):
        raise ValueError("invalid phone number format")
    return phone


def hash_password(password: str, rounds: int = 12) -> str:
    hashed = bcrypt.hashpw(password.encode("utf-8"), bcrypt.gensalt(rounds))
    return hashed.decode("utf-8")


def add_user(
    db_path: Path,
    username: str,
    email: str,
    password: str,
    role: str = "user",
    id_no: str = None,
    name: str = None,
    clearance: str = None,
    institution: str = None,
    specialization: str = None,
    grad_year: int = None,
    phone: str = None,
    warnings: int = 0,
    rounds: int = 12,
):
    username = validate_username(username)
    email = validate_email(email)
    grad_year = validate_graduation_year(grad_year)
    phone = validate_phone(phone)
    if not password:
        raise ValueError("password is required")
    hashed = hash_password(password, rounds=rounds)
    conn = connect(db_path)
    cur = conn.cursor()
    try:
        cur.execute(
            """
            INSERT INTO users
              (id_no, username, name, email, password, clearance_level, institution, specialization, graduation_year, phone, role, account_warnings, created_at)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """,
            (
                id_no,
                username,
                name,
                email,
                hashed,
                clearance,
                institution,
                specialization,
                grad_year,
                phone,
                role,
                warnings if warnings is not None else 0,
                datetime.utcnow().isoformat() + "Z",
            ),
        )
        conn.commit()
        user_id = cur.lastrowid
        print(f"Added user id={user_id} username={username} role={role}")
    except sqlite3.IntegrityError as e:
        print("Error: could not add user — username, email, or id_no may already exist.", file=sys.stderr)
        raise
    finally:
        conn.close()


def list_users(db_path: Path):
    conn = connect(db_path)
    cur = conn.cursor()
    cur.execute(
        """SELECT id, id_no, username, name, email, clearance_level, institution, specialization, graduation_year, phone, role, account_warnings, created_at
           FROM users ORDER BY id"""
    )
    rows = cur.fetchall()
    if not rows:
        print("No users found.")
    else:
        for r in rows:
            print(
                f"id={r['id']}\tid_no={r['id_no']}\tusername={r['username']}\tname={r['name']}\temail={r['email']}\tclearance={r['clearance_level']}\tinstitution={r['institution']}\tspecialization={r['specialization']}\tgrad_year={r['graduation_year']}\tphone={r['phone']}\trole={r['role']}\twarnings={r['account_warnings']}\tcreated_at={r['created_at']}"
            )
    conn.close()


def auth_user(db_path: Path, username: str, password: str):
    conn = connect(db_path)
    cur = conn.cursor()
    cur.execute("SELECT id, password, role FROM users WHERE username = ?", (username,))
    row = cur.fetchone()
    conn.close()
    if not row:
        print("Authentication failed: no such user", file=sys.stderr)
        return False
    stored = row["password"].encode("utf-8")
    ok = bcrypt.checkpw(password.encode("utf-8"), stored)
    if ok:
        print(f"Authenticated: id={row['id']} username={username} role={row['role']}")
    else:
        print("Authentication failed: bad password", file=sys.stderr)
    return ok


def change_password(db_path: Path, username: str, new_password: str = None, rounds: int = 12):
    if new_password is None:
        pw1 = getpass.getpass("New password: ")
        pw2 = getpass.getpass("Confirm password: ")
        if pw1 != pw2:
            raise ValueError("passwords do not match")
        new_password = pw1
    hashed = hash_password(new_password, rounds=rounds)
    conn = connect(db_path)
    cur = conn.cursor()
    cur.execute("UPDATE users SET password = ? WHERE username = ?", (hashed, username))
    if cur.rowcount == 0:
        print("User not found.", file=sys.stderr)
    else:
        conn.commit()
        print(f"Password updated for user {username}")
    conn.close()


def delete_user(db_path: Path, username: str):
    conn = connect(db_path)
    cur = conn.cursor()
    cur.execute("DELETE FROM users WHERE username = ?", (username,))
    if cur.rowcount == 0:
        print("User not found.", file=sys.stderr)
    else:
        conn.commit()
        print(f"Deleted user {username}")
    conn.close()


def parse_args():
    p = argparse.ArgumentParser(description="SQLite users tool (with extra categories)")
    p.add_argument("--db", default=DEFAULT_DB, help="Path to sqlite database (default: ./app.db)")

    sub = p.add_subparsers(dest="cmd", required=True)

    sub_init = sub.add_parser("init", help="Initialize the database (create tables)")

    sub_add = sub.add_parser("add", help="Add a user")
    sub_add.add_argument("--username", required=True)
    sub_add.add_argument("--email", required=True)
    sub_add.add_argument("--password", help="Plaintext password (if omitted, will prompt)")
    sub_add.add_argument("--role", default="user", help="Role (default: user)")
    sub_add.add_argument("--rounds", type=int, default=12, help="bcrypt cost factor (default: 12)")
    # new fields
    sub_add.add_argument("--id-no", dest="id_no", help="ID No (unique)")
    sub_add.add_argument("--name", help="Full name")
    sub_add.add_argument("--clearance", help="Clearance level")
    sub_add.add_argument("--institution", help="Institution")
    sub_add.add_argument("--specialization", help="Specialization")
    sub_add.add_argument("--grad-year", dest="grad_year", type=int, help="Graduation year (e.g. 2020)")
    sub_add.add_argument("--phone", help="Phone number")
    sub_add.add_argument("--warnings", type=int, default=0, help="Account warnings (integer)")

    sub_list = sub.add_parser("list", help="List users")

    sub_auth = sub.add_parser("auth", help="Authenticate a user (prompts for password)")
    sub_auth.add_argument("--username", required=True)

    sub_pass = sub.add_parser("passwd", help="Change user's password (prompts for new password if not provided)")
    sub_pass.add_argument("--username", required=True)
    sub_pass.add_argument("--password", help="New plaintext password (if omitted, will prompt)")
    sub_pass.add_argument("--rounds", type=int, default=12, help="bcrypt cost factor (default: 12)")

    sub_del = sub.add_parser("delete", help="Delete a user")
    sub_del.add_argument("--username", required=True)

    return p.parse_args()


def main():
    args = parse_args()
    db_path = Path(args.db)

    try:
        if args.cmd == "init":
            init_db(db_path)

        elif args.cmd == "add":
            if args.password:
                password = args.password
            else:
                pw1 = getpass.getpass("Password: ")
                pw2 = getpass.getpass("Confirm password: ")
                if pw1 != pw2:
                    print("Error: passwords do not match", file=sys.stderr)
                    sys.exit(2)
                password = pw1
            add_user(
                db_path,
                username=args.username,
                email=args.email,
                password=password,
                role=args.role,
                id_no=args.id_no,
                name=args.name,
                clearance=args.clearance,
                institution=args.institution,
                specialization=args.specialization,
                grad_year=args.grad_year,
                phone=args.phone,
                warnings=args.warnings,
                rounds=args.rounds,
            )

        elif args.cmd == "list":
            list_users(db_path)

        elif args.cmd == "auth":
            pw = getpass.getpass("Password: ")
            ok = auth_user(db_path, args.username, pw)
            if not ok:
                sys.exit(1)

        elif args.cmd == "passwd":
            change_password(db_path, args.username, new_password=args.password, rounds=args.rounds)

        elif args.cmd == "delete":
            confirm = input(f"Delete user {args.username}? This cannot be undone. (yes/NO): ")
            if confirm.lower() == "yes":
                delete_user(db_path, args.username)
            else:
                print("Aborted.")

    except Exception as e:
        print("Error:", e, file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
