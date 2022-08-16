#include <iostream>
#include <vector>
#include <cmath>
#include <cstdio>
#include "util.h"
#include "MBD_distance_2d.h"
#include <queue>
#include <stdio.h>
#include <stdlib.h> 
#include <math.h> 
#include <limits.h>
using namespace std;





void geodesic_shortest_all(const unsigned char * img, const unsigned char * seeds, const unsigned char * destination, unsigned char * shortestpath, 
                              int height, int width)
{
    int * parent = new int[height * width];
    int * distance = new int[height * width];
    vector<queue<Point2D> > Q(20000);
    // point state: 0--acceptd, 1--temporary, 2--far away
    // get initial accepted set and far away set
    int init_dis;
    Point2D start;
    int max_destination =0;
    for(int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            Point2D p;
            p.h = h;
            p.w = w;
            unsigned char seed_type = get_pixel<unsigned char>(seeds, height, width, h, w);
            if(seed_type > 100){
                start.h = h;
                start.w = w;
                init_dis = 0;
                Q[init_dis].push(p);
                set_pixel<int>(distance, height, width, h, w, init_dis);    
                set_pixel<int>(parent, height, width, h, w, h*width+w);  
            }
            else{
                init_dis = 20000;
                set_pixel<int>(distance, height, width, h, w, init_dis);  
                set_pixel<int>(parent, height, width, h, w, height*width);   
            }

            unsigned char tmp_des = get_pixel<unsigned char>(destination, height, width, h, w);
            if (tmp_des > max_destination)
                max_destination = tmp_des;

        }   
    }



    int dh[8] = { 0 , 0 , 1, -1, -1, -1, 1,  1};
    int dw[8] = { 1 , -1, 0,  0, -1, 1 , 1, -1};

    int max_distance = 0;

    std::vector<int> check(max_destination-1, 0);



    // Proceed the propagation from the marker to all pixels in the image
    for (int lvl = 0; lvl < 20000; lvl++)
    {
        while (!Q[lvl].empty())
        {
            Point2D p = Q[lvl].front();
            Q[lvl].pop();

            unsigned char stop_type = get_pixel<unsigned char>(destination, height, width, p.h, p.w);
		
            if(stop_type >1)
		        check[stop_type -2] = 1;
	        int product = 1;
	        for (int i = 0; i < max_destination-1; i++)
		        product = product * check[i];
            if (product == 1)
                goto endloop;
            else
            {
                for (int n1 = 0 ; n1 < 8 ; n1++)
                {
                    int tmp_h  = p.h + dh[n1];
                    int tmp_w  = p.w + dw[n1];

                    if (tmp_h >= 0 and tmp_h < height and tmp_w >= 0 and tmp_w < width)
                    {
                        Point2D r;
                        r.h = tmp_h;
                        r.w = tmp_w;
                        float dt_space = sqrt(dh[n1] * dh[n1] + dw[n1] * dw[n1]);
                        
                        int temp_r = get_pixel<int>(distance, height, width,  r.h, r.w);
                        int temp_p = get_pixel<int>(distance, height, width,  p.h, p.w);
                        unsigned char tmp_img_r = get_pixel<unsigned char>(img, height, width, r.h, r.w);
                        unsigned char tmp_img_p = get_pixel<unsigned char>(img, height, width, p.h, p.w);  
                        int tmp_dis = temp_p + 10* int(abs(tmp_img_r - tmp_img_p)) + int(dt_space);

                        if (temp_r > tmp_dis)
                        {
                            set_pixel<int>(distance, height, width, r.h, r.w, tmp_dis);
                            set_pixel<int>(parent, height, width, r.h, r.w, p.h* width + p.w);
                            Q[tmp_dis].push(r);
                        }
                        if (max_distance < tmp_dis)
                    max_distance = tmp_dis;
                    }
                }
	        }	
        }
    }
    
    endloop:


    for (int h = 0; h < height ; h++)
    {
        for (int w = 0; w < width ; w++)
        {
            set_pixel<unsigned char>(shortestpath, height, width, h, w, 0);
        }
    }
    // cout << "max_destination:  " << max_value << endl;

    // trace back


    for(int t = 2; t < max_destination + 1; t++)
    {
        unsigned char * shortestpath_tmp = new unsigned char[height * width];
        Point2D des;
        for(int h = 0; h < height; h++)
        {
            for (int w = 0; w < width; w++)
            {
                Point2D p;
                p.h = h;
                p.w = w;
                unsigned char des_type = get_pixel<unsigned char>(destination, height, width, h, w);
                if(des_type == t)
                {
                    des = p;
                    set_pixel<unsigned char>(shortestpath_tmp, height, width, p.h, p.w, t);
                    set_pixel<unsigned char>(shortestpath, height, width, p.h, p.w, t);

                }
                else
                {
                    set_pixel<unsigned char>(shortestpath_tmp, height, width, p.h, p.w,0);
                }          
            }
        }
        
        Point2D p = des;

        int par = get_pixel<int>(parent, height, width, p.h, p.w);
        while(par != start.h * width + start.w)
        {
            set_pixel<unsigned char>(shortestpath_tmp, height, width, p.h, p.w,t);
            set_pixel<unsigned char>(shortestpath, height, width, p.h, p.w,t);
            int tmp_0 = int(floor(par/width));
            int tmp_1 = par % width;
            p.h = tmp_0;
            p.w = tmp_1;
            par = get_pixel<int>(parent, height, width, p.h, p.w);
            // std::cout << tmp_0 << "  " << tmp_1 << std::endl;
        }
        delete shortestpath_tmp;

    }


    delete parent;
    delete distance;
}


