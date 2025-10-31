#!/usr/bin/env python3
"""
Import users from SQL file into the Sohbet database
Maps SQL table fields to our User schema
"""

import sqlite3
import re
import bcrypt
import sys

def hash_password(password: str) -> str:
    """Hash a password using bcrypt"""
    salt = bcrypt.gensalt()
    return bcrypt.hashpw(password.encode('utf-8'), salt).decode('utf-8')

def import_users(sql_file: str, db_file: str):
    """Import users from SQL file into database"""
    
    # Read SQL file
    with open(sql_file, 'r', encoding='utf-8') as f:
        sql_content = f.read()
    
    # Extract INSERT statements
    insert_pattern = r"INSERT INTO mytable\([^)]+\) VALUES \(([^)]+)\);"
    matches = re.findall(insert_pattern, sql_content)
    
    if not matches:
        print("No INSERT statements found in SQL file")
        return
    
    print(f"Found {len(matches)} users to import")
    
    # Connect to database
    conn = sqlite3.connect(db_file)
    cursor = conn.cursor()
    
    imported = 0
    skipped = 0
    
    for match in matches:
        # Parse values (handle quoted strings and numbers)
        values = []
        current = ""
        in_quotes = False
        
        for char in match + ',':
            if char == "'" and not in_quotes:
                in_quotes = True
            elif char == "'" and in_quotes:
                in_quotes = False
            elif char == ',' and not in_quotes:
                values.append(current.strip())
                current = ""
            else:
                current += char
        
        if len(values) < 11:
            print(f"Skipping invalid row: {match[:50]}...")
            skipped += 1
            continue
        
        # Map SQL fields to our schema
        # SQL: ID No, Name, Username, Position, Institution, Specialization, 
        #      Graduation Year, Phone No, Email, Creation, Warnings
        user_id = int(values[0])
        name = values[1].strip("'")
        username = values[2].strip("'")
        position = values[3].strip("'")
        university = values[4].strip("'")
        department = values[5].strip("'")
        graduation_year = int(values[6])
        phone = values[7].strip("'")
        email = values[8].strip("'")
        created_at = values[9].strip("'")
        warnings = int(values[10])
        
        # Generate a default password (in production, users would set their own)
        default_password = f"{username}123"
        password_hash = hash_password(default_password)
        
        try:
            # Insert user
            cursor.execute("""
                INSERT INTO users (
                    id, username, email, password_hash, name, position, phone_number,
                    university, department, enrollment_year, warnings,
                    primary_language, created_at
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """, (
                user_id, username, email, password_hash, name, position, phone,
                university, department, graduation_year, warnings, 'Turkish', created_at
            ))
            imported += 1
            print(f"✓ Imported: {name} ({username})")
        except sqlite3.IntegrityError as e:
            print(f"✗ Skipped {username}: {e}")
            skipped += 1
    
    conn.commit()
    conn.close()
    
    print(f"\n=== Import Summary ===")
    print(f"Total users in SQL file: {len(matches)}")
    print(f"Successfully imported: {imported}")
    print(f"Skipped: {skipped}")
    print(f"\nDefault password for all users: [username]123")
    print(f"Example: ahmet_yilmaz password is 'ahmet_yilmaz123'")

if __name__ == "__main__":
    sql_file = "attached_assets/Sohbet_1761555335301.sql"
    db_file = "build/academic.db"
    
    print("=== Sohbet User Import Tool ===\n")
    import_users(sql_file, db_file)
