import sys
import numpy as np
import argparse

parser = argparse.ArgumentParser(description='launch exercise tool.')
parser.add_argument('initialcases', type=str, help='initial cases XML filename')
parser.add_argument('parameters', type=str, help='parameters XML filename')
parser.add_argument('days', type=int, help='number of days (time steps)')
parser.add_argument('numPerScreen', type=int, help='number of processes per screen')

args = parser.parse_args()

nodes = range(1, 20+1, 1)
screens = range(4)

index = 0

for n in nodes:
    for s in screens:
        screenLines = '( '

        for j in range(args.numPerScreen):
            screenLine = 'ssh s' + "{0:02d}".format(n) + ' DISPLAY=:0 `which exercise` -geometry 2560x1600+' + str(2560*s) + '+0 --batch --batch-numtimesteps ' + str(args.days) + ' --batch-initialcasesfilename $PWD/' + args.initialcases + ' --batch-parametersfilename $PWD/' + args.parameters + ' --batch-outputvariable deceased --batch-outputfilename $PWD/deceased-' + str(index) + '.csv > /dev/null 2>&1'

            if j < args.numPerScreen-1:
                screenLines += screenLine + '; '
            else:
                screenLines += screenLine + ' ) &'

            index = index + 1

        print screenLines
