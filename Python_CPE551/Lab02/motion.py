# Author: Guillaume Wantiez
# Date: 01/31/2026 (1:05 PM)
# Description: Given a duration of time, this program computes 
# the velocity, average velocity, and displacement of an object.

# Useful values:
acceleration = 5.25
initialVelocity = 8.25

# Initialize the radius:
time = 10.0

# Calculate the properties of the object:
finalVelocity = initialVelocity + (acceleration * time)
averageVelocity = initialVelocity + (1/2)*(acceleration * time)
displacement = (initialVelocity*time) + (1/2)*(acceleration*(time*time))

# Print the results:
print(f"time = {time}\n")
print(f"velocity         = {finalVelocity}")
print(f"average velocity = {averageVelocity}")
print(f"displacement     = {displacement}")