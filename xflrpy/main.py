import random
miarex = MiarexWrapper
plane = miarex.getPlane()
analysis = plane.getAnalysis(0)

plane.resetView(False)
analysis.showDialog(False)
# analysis.setSeq(0,10,0.5)

def randrange(a, b):
    return a + (b-a)*random.random()

sections = (5, 8) # (wing, winglet)

for i in range(40):
    # section = int(randrange(0,3))
    section = random.sample(sections, 1)[0]
    taper= randrange(0.4,0.6)
    if section==8: taper+=0.2
    span = randrange(0.45,0.575)
    sweep = int(randrange(20,40))

    # plane.setSpan(span, 1, 0)
    plane.setTaper(taper, section, 0) # (value, section, wing)
    plane.setSweep(sweep, section, 0)
    
    polar = analysis.analyze()
    CLCD = polar.getCLCD(10) # arg = alpha
    print(f"CL/CD for plane {i} = {CLCD} : (section: {section}, taper: {taper}, sweep: {sweep}, span: {span})")
