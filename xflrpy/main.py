import random
miarex = MiarexWrapper
plane = miarex.get_plane('api_test')
analysis = plane.get_analysis(0)
analysis.setSeq(0,10,0.5)

def randrange(a, b):
    return a + (b-a)*random.random()

for i in range(10):
    chord = randrange(0.1,0.4) # metres
    plane.setChord(chord)
    polar = analysis.analyze()
    CLCD = polar.getCLCD(10)
    print(i, chord, CLCD)
