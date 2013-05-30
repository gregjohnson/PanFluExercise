#ifndef STOCKPILE_H
#define STOCKPILE_H

#include <QtGui>
#include <string>
#include <vector>
#include <boost/array.hpp>

enum STOCKPILE_TYPE { STOCKPILE_ANTIVIRALS, STOCKPILE_VACCINES, NUM_STOCKPILE_TYPES };

class Stockpile : public QObject
{
    Q_OBJECT

    public:

        Stockpile(std::string name);

        static std::string getTypeName(STOCKPILE_TYPE type);

        std::string getName();

        int getNum(int time, STOCKPILE_TYPE type);

        void setNodeIds(std::vector<int> nodeIds);
        std::vector<int> getNodeIds();

        void copyToNewTimeStep();

    public slots:

        void setNum(int time, int num, STOCKPILE_TYPE type);

    private:

        // name for the stockpile
        std::string name_;

        // number of available resource at each timestep
        std::vector<boost::array<int, NUM_STOCKPILE_TYPES> > num_;

        // nodeIds serviced from this stockpile
        std::vector<int> nodeIds_;
};

#endif
