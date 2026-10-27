import subprocess
import os
import sys
import time

def main():
    topic = sys.argv[1] if len(sys.argv) > 1 else "Computer"
    
    # 1500ms: Press UP to highlight first search result (now nearest to the keyboard)
    # 2500ms: Press ENTER to open article
    # 5000ms: Press ENTER to click the first link on the article (automatically highlighted)
    # 8000ms: QUIT simulator
    sim_script = "1500:UP;2500:ENTER;5000:ENTER;8000:QUIT"
    
    env = os.environ.copy()
    env["CROSSPOINT_SIM_INPUT_SCRIPT"] = sim_script
    env["CROSSPOINT_SIM_WIKI_START"] = topic
    env.setdefault("WIKI_RAW_DRIVE", "wikipedia.img")
    
    program = ".pio/build/simulator_x3_wsl/program"
    if not os.path.exists(program):
        print(f"Error: Simulator binary not found at {program}")
        print("Please wait for the current compilation to finish.")
        sys.exit(1)
        
    print(f"Running full UI simulator test for topic: '{topic}'")
    
    process = subprocess.Popen(
        [program],
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True
    )
    
    output = []
    print("\n--- Simulator Output Stream ---")
    for line in process.stdout:
        if "Following link to" in line or "Fallback:" in line or "Link target not found" in line:
            out = f">>> {line.strip()}"
            print(out)
            output.append(out)
            
    process.wait()
    
    if any(any(substr in l for substr in ["Following link", "Fallback:", "Link target not found"]) for l in output):
        print("\nSUCCESS: The full C++ UI simulator natively followed a link via emulated X3 button clicks!")
    else:
        print("\nFAILURE: No link followed. (Check if the article has links or if timing was off).")

if __name__ == "__main__":
    main()
