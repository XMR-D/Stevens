import random

def pickRandomWord():
    words = [
        "apple", "brave", "chair", "dream", "eagle",
        "flame", "grape", "house", "jolly", "knife",
        "light", "magic", "north", "ocean", "pride",
        "quiet", "river", "stone", "tiger", "waltz",
        "absent", "beacon", "candle", "dragon", "empire",
        "forest", "galaxy", "hammer", "island", "jungle",
        "kitten", "ladder", "mirror", "napkin", "orange",
        "planet", "quartz", "rocket", "silver", "travel",
        "academy", "balance", "captain", "diamond", "emotion",
        "freedom", "gallery", "harvest", "imagine", "journey",
        "kingdom", "library", "mystery", "natural", "orchard",
        "passion", "quality", "respect", "science", "teacher"
    ]
    return random.choice(words)