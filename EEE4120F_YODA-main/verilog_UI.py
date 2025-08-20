import subprocess
import time

key = input("Enter a 4 character key (4 char max): \n")
while( len(key) > 4 or len(key) < 1):
    print("Key must be between 1 and 4 characters. \n")
    key = input("Enter a 4 character key (4 char max): \n")

print()
message = input("Enter a message to encrypt: (1023 char max): \n")
print()
while( len(message) > 1023 or len(message) < 1):
    print("Message must be between 1 and 1023 characters. \n")
    message = input("Enter a message to encrypt: (1023 char max): \n")


with open("data.txt", "w") as f:
    f.write(key + "\n")
    f.write(message + "\n")
    
# Compile
#subprocess.run(["iverilog", "-o", "design.out", "DEA_tb.v", "DEA.v", "key_reg.v"])
subprocess.run(["iverilog", "-o", "design.out", "UI_tb.v", "DEA.v", "key_reg.v"], capture_output=True, text=True)


# Run simulation
result = subprocess.run(["vvp", "design.out"], capture_output=True, text=True)

print()
print("Encrypted Message:")
with open("output.txt", "r") as file:
    contents = file.read()
print()
for c in contents:
    print(c, end='', flush=True)
    time.sleep(0.1)
