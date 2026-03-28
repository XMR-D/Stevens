class Music:

    def __init__(self, id=0, name=None, album_name=None, artist_name=None, genre=None, nb_songs=0, nb_copies=0):
        self.__id = id
        self.__album_name = album_name
        self.__artist_name = artist_name
        self.__genre = genre
        self.__nb_songs = nb_songs
        self.__nb_copies = nb_copies


    def GetId(self):
        return self.__id

    def GetAlbumName(self):
        return self.__album_name
    
    def GetArtistName(self):
        return self.__artist_name
    
    def GetGenre(self):
        return self.__genre
        
    def GetNbSongs(self):
        return self.__nb_songs
    
    def GetNbCopies(self):
        return self.__nb_copies
    

    def AddCopy(self):
        self.__nb_copies += 1

    def RmCopy(self):
        self.__nb_copies -= 1
