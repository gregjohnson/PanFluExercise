#ifndef STOCKPILE_H
#define STOCKPILE_H

#include <QtGui>
#include <string>
#include <vector>

class Stockpile : public QObject
{
    Q_OBJECT

    public:

        Stockpile(std::string name);

        std::string getName();

        int getNum(int time);

        void setNodeIds(std::vector<int> nodeIds);
        std::vector<int> getNodeIds();

        void copyToNewTimeStep();

    signals:

        void changed();

    public slots:

        void setNum(int time, int num);

    private:

        // name for the stockpile
        std::string name_;

        // number of available resource at each timestep
        std::vector<int> num_;

        // nodeIds serviced from this stockpile
        std::vector<int> nodeIds_;
};

#endif
