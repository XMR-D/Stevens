class Movies:
    def __init__(self, ID, movieName, genre, length, numCopies):
        self.ID = ID
        self.movieName = movieName
        self.genre = genre
        self.length = length
        self.numCopies = numCopies

    def getID(self):
        return self.ID

    def getMovieName(self):
        return self.movieName

    def getGenre(self):
        return self.genre

    def getLength(self):
        return self.length

    def getNumCopies(self):
        return self.numCopies

    def setNumCopies(self, numCopies):
        self.numCopies = numCopies
