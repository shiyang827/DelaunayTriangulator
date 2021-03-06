/****************************************************************************
Copyright 2017, Cognitics Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy 
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights 
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom the Software is 
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in 
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
THE SOFTWARE.
****************************************************************************/

#include <stdio.h>
#include <vector>
#include <string>

#include <iostream>

#include "ctl.h"

// To make this simple, we'll return the polygon as a list of rings (each a pointlist)
// The first ring is the outer, the rest are interior rings (aka holes)
std::vector<ctl::PointList> generatePolygon()
{   
   std::vector<ctl::PointList> poly;
   ctl::PointList outer;
   outer.push_back(ctl::Point(20,70));
   outer.push_back(ctl::Point(20,20));
   outer.push_back(ctl::Point(40,50));
   outer.push_back(ctl::Point(60,30));
   outer.push_back(ctl::Point(70,70));
   outer.push_back(ctl::Point(80,80));
   outer.push_back(ctl::Point(80,60));
   outer.push_back(ctl::Point(100,50));
   outer.push_back(ctl::Point(100,80));
   outer.push_back(ctl::Point(160,110));
   outer.push_back(ctl::Point(100,150));
   outer.push_back(ctl::Point(30,120));
   outer.push_back(ctl::Point(20,70));

   ctl::PointList inner;
   inner.push_back(ctl::Point(80,100));
   inner.push_back(ctl::Point(80,120));
   inner.push_back(ctl::Point(60,120));
   inner.push_back(ctl::Point(60,100));
   inner.push_back(ctl::Point(80, 100));
   
   poly.push_back(outer);
   poly.push_back(inner);
   return poly;
}


int main(int argc, char **argv)
{
   // Construct the outer bounds of the area to be triangulated. Ideally this 
   // should be the Minimum bounding rectangle of the polygon
   double minX = 0;
   double minY = 0;
   double maxX = 160;
   double maxY = 150;
   ctl::PointList boundary;
   boundary.push_back(ctl::Point(minX, minY));
   boundary.push_back(ctl::Point(maxX, minY));
   boundary.push_back(ctl::Point(maxX, maxY));
   boundary.push_back(ctl::Point(minX, maxY));
   ctl::DelaunayTriangulation *dt = new ctl::DelaunayTriangulation(boundary);

   std::vector<ctl::PointList> poly = generatePolygon();
   dt->InsertConstrainedLineString(poly.at(0));
   for (size_t i = 1, ic = poly.size(); i < ic; i++)
   {
      dt->InsertConstrainedLineString(poly.at(i));
   }

   ctl::TIN tin(dt, dt->GatherTriangles(poly.at(0)));

   for (size_t i = 0, c = tin.triangles.size(); i < c; i += 3)
   {
      //Test if the centroid of the triangle is inside the outer ring
      //and not inside the inner rings
      ctl::Point centroid(
         (tin.verts[tin.triangles[i + 0]].x +
         tin.verts[tin.triangles[i + 1]].x +
         tin.verts[tin.triangles[i + 2]].x) / 3,
         (tin.verts[tin.triangles[i + 0]].y +
         tin.verts[tin.triangles[i + 1]].y +
         tin.verts[tin.triangles[i + 2]].y) / 3,
         (tin.verts[tin.triangles[i + 0]].z +
         tin.verts[tin.triangles[i + 1]].z +
         tin.verts[tin.triangles[i + 2]].z) / 3
         );

      bool isInOutline = ctl::PointInPolygon(centroid, poly.at(0));
      if (!isInOutline)
            std::cout << "ERROR: Triangle is not inside the outline" << std::endl;

      bool keepTriangle = true;

      for (size_t j = 1, jc = poly.size(); j < jc; j++)
      {
         // Discard any triangle whose centroid is inside a hole
         if (ctl::PointInPolygon(centroid, poly.at(j)))
         {
            keepTriangle = false;
            break;
         }         
      }
      if (keepTriangle)
      {
         std::cout << "Triangle: " << i << std::endl;
         std::cout << "\t" << tin.verts[tin.triangles[i + 0]].x << ", " <<
            tin.verts[tin.triangles[i + 0]].y << ", " <<
            tin.verts[tin.triangles[i + 0]].z << std::endl;

         std::cout << "\t" << tin.verts[tin.triangles[i + 1]].x << ", " <<
            tin.verts[tin.triangles[i + 1]].y << ", " <<
            tin.verts[tin.triangles[i + 1]].z << std::endl;

         std::cout << "\t" << tin.verts[tin.triangles[i + 2]].x << ", " <<
            tin.verts[tin.triangles[i + 2]].y << ", " <<
            tin.verts[tin.triangles[i + 2]].z << std::endl;
         std::cout << std::endl;
      }
   }
   delete dt;

	return 0;
}