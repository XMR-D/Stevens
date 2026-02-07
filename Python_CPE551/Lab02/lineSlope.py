# Author: Guillaume Wantiez
# Date: 01/31/2026 (1:00 PM)
# Description: This program computes the slope of a line given 
# the end points of the line. The result is then printed to the screen.


# Initialize the end points.
startX = -2
startY = 1
endX = 5
endY = 36

# Compute the slope.
slope = (startY - endY) / (startX - endX)

# Print the results.
print("Starting point: (", startX, ",", startY, ")")
print("Ending point: (", endX, ",", endY, ")")
print(f"Slope of the line = {slope}")

#BUG REPORT
#Syntax bug found : 
# l.14 Extra indentation + startx/endx not defined (startX/endX should be used here)
# l.17 Missing a ',' after third parameter
# l.19 Missing a closing parenthesis on print statement + extra ')' (maybe meant to be outside the string)

#Logic bug found : 
# l.14, Missing parenthesis for slope computation for ops priority rules
#       + denominator computation was inverted leading to negative result
# l.19, slope refers to a variable hence print string should start
#       with 'f' for formating values and slope should be lowercased to refer to the variable