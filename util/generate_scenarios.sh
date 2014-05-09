# $ python generate_batch_commands.py randomall parameters.xml 1 2 5 4 3 2 1 10 5 test1 -h
# usage: generate_batch_commands.py [-h]
#                                   initialcasestype parameters R0range R0range
#                                   CFRs CFRs CFRs CFRs CFRs days num outputdir
# 
# launch exercise tool.
# 
# positional arguments:
#   initialcasestype  initial cases type (randomall, populationweightedall,
#                     bordercrossingweighted)
#   parameters        input parameters XML filename
#   R0range           minimum R0
#   CFRs              case fatality rates (5 age groups)
#   days              number of days (time steps)
#   num               number of runs
#   outputdir         output directory
# 
# optional arguments:
#   -h, --help        show this help message and exit

# ic1 - random all
# ic2 - random all, population weighted
# ic3 - random border counties, weighted by migrating population

# C1: 0.000092 0.000168 0.000343 0.000253 0.000037 1.4 1.55
mkdir c1
mkdir c1/ic1
mkdir c1/ic2
mkdir c1/ic3
python generate_batch_commands.py randomall                parameters.xml   1.4 1.55   0.000092 0.000168 0.000343 0.000253 0.000037   450   150 c1/ic1
python generate_batch_commands.py populationweightedall    parameters.xml   1.4 1.55   0.000092 0.000168 0.000343 0.000253 0.000037   450   150 c1/ic2
python generate_batch_commands.py bordercrossingweighted   parameters.xml   1.4 1.55   0.000092 0.000168 0.000343 0.000253 0.000037   450   150 c1/ic3


# C2: 0.000096 0.000134 0.0002215 0.0013765 0.0100185 1.55 1.75
mkdir c2
mkdir c2/ic1
mkdir c2/ic2
mkdir c2/ic3
python generate_batch_commands.py randomall                parameters.xml   1.55 1.75   0.000096 0.000134 0.0002215 0.0013765 0.0100185   450   150 c2/ic1
python generate_batch_commands.py populationweightedall    parameters.xml   1.55 1.75   0.000096 0.000134 0.0002215 0.0013765 0.0100185   450   150 c2/ic2
python generate_batch_commands.py bordercrossingweighted   parameters.xml   1.55 1.75   0.000096 0.000134 0.0002215 0.0013765 0.0100185   450   150 c2/ic3


# C3: 0.0001 0.0001 0.0001 0.0025 0.02 1.55 1.75
mkdir c3
mkdir c3/ic1
mkdir c3/ic2
mkdir c3/ic3
python generate_batch_commands.py randomall                parameters.xml   1.55 1.75   0.0001 0.0001 0.0001 0.0025 0.02   450   150 c3/ic1
python generate_batch_commands.py populationweightedall    parameters.xml   1.55 1.75   0.0001 0.0001 0.0001 0.0025 0.02   450   150 c3/ic2
python generate_batch_commands.py bordercrossingweighted   parameters.xml   1.55 1.75   0.0001 0.0001 0.0001 0.0025 0.02   450   150 c3/ic3


# C4: 0.0087 0.0011 0.003 0.0098 0.0403 1.75 2.3
mkdir c4
mkdir c4/ic1
mkdir c4/ic2
mkdir c4/ic3
python generate_batch_commands.py randomall                parameters.xml   1.75 2.3   0.0087 0.0011 0.003 0.0098 0.0403   450   150 c4/ic1
python generate_batch_commands.py populationweightedall    parameters.xml   1.75 2.3   0.0087 0.0011 0.003 0.0098 0.0403   450   150 c4/ic2
python generate_batch_commands.py bordercrossingweighted   parameters.xml   1.75 2.3   0.0087 0.0011 0.003 0.0098 0.0403   450   150 c4/ic3


# C5: 0.0196 0.0156 0.016 0.0068 0.0151 1.75 2.3
mkdir c5
mkdir c5/ic1
mkdir c5/ic2
mkdir c5/ic3
python generate_batch_commands.py randomall                parameters.xml   1.75 2.3   0.0196 0.0156 0.016 0.0068 0.0151   450   150 c5/ic1
python generate_batch_commands.py populationweightedall    parameters.xml   1.75 2.3   0.0196 0.0156 0.016 0.0068 0.0151   450   150 c5/ic2
python generate_batch_commands.py bordercrossingweighted   parameters.xml   1.75 2.3   0.0196 0.0156 0.016 0.0068 0.0151   450   150 c5/ic3
