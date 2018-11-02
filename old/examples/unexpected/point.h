#ifndef __POINT_H
#define __POINT_H

#include <mpi.h>

class point
{
  public:
    point() {}
    point(int xp, int yp) :x(xp),y(yp) {}
    int getIter() {
       	return iter;
    }
    void setIter(int i) {
    	iter = i;
    }

    friend void write(int epoch, int rank, point& pt) {
    	std::ofstream t;
    	char buf[20];
    	sprintf(buf, "check_%d_%d", epoch, rank);
    	t.open(buf, std::ios::app);
    	std::cout << "In file write\n";
    	t.write(reinterpret_cast<char *>(&pt.getIter()), sizeof(int));
    	t.write(reinterpret_cast<char *>(&pt.x), sizeof(int));
    	t.write(reinterpret_cast<char *>(&pt.y), sizeof(int));

    	t.close();
    }
    friend void read(int epoch, int rank, point& pt) {
    	std::ifstream t;
    	char buf[20];
    	sprintf(buf, "check_%d_%d", epoch, rank);
    	t.open(buf, std::ios::in);
    	std::cout << "In file read\n";
    	int val;
    	t.read(reinterpret_cast<char *>(&val), sizeof(int));
    	pt.setIter(val);
    	t.read(reinterpret_cast<char *>(&val), sizeof(int));
    	pt.x = val;
    	t.read(reinterpret_cast<char *>(&val), sizeof(int));
    	pt.y = val;

    	t.close();
    }
    MPI_Request *rreq;
    MPI_Request *sreq;
    int x, y;
  private:
    int iter;
};

#endif
