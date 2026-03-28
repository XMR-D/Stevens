class Book:

    def __init__(self, id=0, name="Default", artist=None, genre=None, nb_songs=0, nb_copies=0):
        self.__id = id
        self.__book_name = name
        self.__author_name = artist
        self.__genre = genre
        self.__nb_pages = nb_songs
        self.__nb_copies = nb_copies


    def GetId(self):
        return self.__id

    def GetName(self):
        return self.__book_name
    
    def GetAuthorName(self):
        return self.__author_name
    
    def GetGenre(self):
        return self.__genre
        
    def GetNbPages(self):
        return self.__nb_pages
    
    def GetNbCopies(self):
        return self.__nb_copies
    

    def AddCopy(self):
        self.__nb_copies += 1

    def RmCopy(self):
        self.__nb_copies -= 1
