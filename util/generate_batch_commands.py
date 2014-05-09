import sys
import random
import numpy as np
import xml.etree.ElementTree as ET
import argparse

parser = argparse.ArgumentParser(description='launch exercise tool.')
parser.add_argument('initialcasestype', type=str, help='initial cases type (randomall, populationweightedall, bordercrossingweighted)')
parser.add_argument('parameters', type=str, help='input parameters XML filename')
parser.add_argument('R0range', type=float, nargs=2, help='minimum R0')
parser.add_argument('CFRs', type=float, nargs=5, help='case fatality rates (5 age groups)')
parser.add_argument('days', type=int, help='number of days (time steps)')
parser.add_argument('num', type=int, help='number of runs')

args = parser.parse_args()

# static parameters
numCountiesMin = 1
numCountiesMax = 10

numCasesPerCountyMin = 1
numCasesPerCountyMax = 20

# load population data for generation of initial cases
data = np.genfromtxt('scenario_counties-' + args.initialcasestype + '.csv', delimiter=',', names=True)

for i in range(args.num):
    # filenames for this case
    runInitialCasesFilename = 'initialcases-' + '{0:03d}'.format(i) + '.xml'
    runParametersFilename = 'parameters-' + '{0:03d}'.format(i) + '.xml'
    outputFilename = 'treatable-' + '{0:03d}'.format(i) + '.csv'

    # generate initial cases XML
    root = ET.fromstring('<root> </root>')

    # select counties (sampling without replacement, weighted based on weight column)
    countiesFips = np.random.choice(data['fips'], size=random.randint(numCountiesMin, numCountiesMax), replace=False, p=data['weight'] / np.sum(data['weight']))

    for fips in countiesFips:
        numCases = random.randint(numCasesPerCountyMin, numCasesPerCountyMax)

        sub = ET.SubElement(root, 'cases')
        sub.set('num', str(numCases))
        sub.set('nodeId', str(int(fips)))

    tree = ET.ElementTree(root)
    tree.write(runInitialCasesFilename)

    # random R0 within range
    R0 = random.uniform(args.R0range[0], args.R0range[1])

    # modify parameters XML for new R0 and CFRs
    tree = ET.parse(args.parameters)
    root = tree.getroot()

    elementR0 = root.findall('.//R0')

    for e in elementR0:
        e.set('value', str(R0))

    elementCFRs = root.findall('.//caseFatalityRates')

    for e in elementCFRs:
        e.set('value0', str(args.CFRs[0]))
        e.set('value1', str(args.CFRs[1]))
        e.set('value2', str(args.CFRs[2]))
        e.set('value3', str(args.CFRs[3]))
        e.set('value4', str(args.CFRs[4]))

    # write scenario parameters file
    tree.write(runParametersFilename)

    # batch command
    line = 'DISPLAY=:0 exercise --batch --batch-numtimesteps ' + str(args.days) + ' --batch-initialcasesfilename ' + runInitialCasesFilename + ' --batch-parametersfilename ' + runParametersFilename + ' --batch-outputvariable treatable --batch-outputfilename ' + outputFilename + ' > /dev/null 2>&1'

    print line
