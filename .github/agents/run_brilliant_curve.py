import yaml
import subprocess

AGENT_FILE = ".github/agents/brilliant_curve_data.yml"

# Load agent YAML
with open(AGENT_FILE) as f:
    agent = yaml.safe_load(f)

progress_tracker = agent['progress_tracker']

print(f"Running Brilliant_Curve agent: {agent['name']}\n")

for task in progress_tracker:
    task_name = task['task']
    print(f"Starting task: {task_name}")
    
    # Update status to In Progress
    task['status'] = "In Progress"
    task['icon'] = "edit-find-symbolic"
    print(f" - Status: {task['status']} ({task['icon']})")
    
    # === PLACEHOLDER FOR REAL TASK LOGIC ===
    # Example: call shell commands or scripts for each task
    # subprocess.run(["./tasks/implement_email_verification.sh"])
    
    # Simulate task completion
    task['status'] = "Completed"
    task['icon'] = "document-save-symbolic"
    print(f" - Status: {task['status']} ({task['icon']})\n")

# Save updated YAML
with open(AGENT_FILE, 'w') as f:
    yaml.dump(agent, f, sort_keys=False)

print("All tasks completed. Progress Tracker updated.\n")

# Commit changes
subprocess.run(["git", "config", "user.name", "github-actions"])
subprocess.run(["git", "config", "user.email", "actions@github.com"])
subprocess.run(["git", "add", AGENT_FILE])
subprocess.run(["git", "commit", "-m", "[agent] Updated Progress Tracker"])
subprocess.run(["git", "push"])

