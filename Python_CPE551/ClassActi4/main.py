import music
import movie
import books

def main():

    with open("", "r") as file:
        for line in file:
            for words in line.strip().split(','):
                #DO THINGS
                match(words[1]):
                    case "Music":
                        continue

                    case "Movie":
                        continue
                    
                    case "Book":
                        continue

    Misty_montain = music.Music(0, "Jean-claude", "Misty_montain", "pop", 1, 1) 
    the_lord_of_the_ring = movie.Movies(0, "the_lord_of_the_ring", "Fantasy", 180, 1)
    The_Simarillion = books.Book()

    print(Misty_montain.GetAlbumName())

    the_lord_of_the_ring.setNumCopies(10)
    print(the_lord_of_the_ring.getNumCopies())

    print(The_Simarillion.GetName())


main()