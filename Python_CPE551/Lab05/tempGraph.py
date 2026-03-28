# Author: Guillaume Wantiez
# Date: 02/23/26
# Description: Generates and plots hourly temperature data for Paris, New York, 
# and Melbourne, ensuring data dimensions are synchronized for accurate visualization.

import matplotlib.pyplot as plt
import random as rand

time = [i for i in range(13)]

#PARIS
Paris_L = [rand.randint(10, 30) for i in range(13)]
plt.plot(time, Paris_L, label="Paris")

#NEWYORK
NewYork_L = [rand.randint(10, 30) for i in range(13)]
plt.plot(time, NewYork_L, label="NewYork")

#MELBOURNE
Melbourne_L = [rand.randint(10, 30) for i in range(13)]
plt.plot(time, Melbourne_L, label="Melbourne")


# Adding the title
plt.title("Hourly Temperatures")

# Adding the labels
plt.xlabel("Hours")
plt.ylabel("Temperature")

plt.legend()


plt.show()