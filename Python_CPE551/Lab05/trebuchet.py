# Author: Guillaume Wantiez
# Date: 02/23/26
# Description: Tracks trebuchet launch distances and maintains a sorted list 
# of the top three performances with their corresponding trial numbers.

def update_top_dist(trial, dist, top_dist, trial_nb):
    i = 0
    while ((i < 3) and (dist <= top_dist[i])):
        i += 1
    
    while(i != 3):
        saved_trial = trial_nb[i]
        saved_dist = top_dist[i]
        trial_nb[i] = trial
        top_dist[i] = dist
        dist = saved_dist
        trial = saved_trial
        i += 1

    
def print_top_dist(tria_nb, top_dist):
    print("Trial Distance")
    for i in range(3):
        print(f"{tria_nb[i]:<8} {top_dist[i]:>10}")


def main():

    top_distances = [0, 0, 0]
    trial_numbers = [0, 0, 0]
    extra_trial_flag = "Y"
    current_trial = 1

    while (extra_trial_flag == "Y"):
        dist = input(f"Please enter your distance for trial {current_trial}: ")
        update_top_dist(current_trial, int(dist), top_distances, trial_numbers)
        extra_trial_flag = input(f"Would you like to input another trial? (Y/N): ")
        current_trial += 1

    print("The top three distances for the trebuchet are:")
    print_top_dist(trial_numbers, top_distances)


if __name__ == "__main__":
    main()