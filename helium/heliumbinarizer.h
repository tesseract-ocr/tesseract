// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the HeliumBinarizer class, used to binarize given
// Clusters in a given Image. Its main strengths are the use of color 
// information, which allows the binarizer to search for components, that the
// text detector might have missed.
//
#ifndef HELIUM_MOBILEBINARIZER_H__
#define HELIUM_MOBILEBINARIZER_H__

#include "array.h"
#include "binarizer.h"
#include "box.h"
#include "color.h"
#include "clusterer.h"
#include "mask.h"
#include "unionfind.h"

namespace helium {

class Shape;

// The HeliumBinarizer class contains a complex set of data structures and
// methods to perform binarization and box extension on a given image with
// a given set of Clusters. 
// Most of the work is actually done before binarization, when Clusters are
// added to the HeliumBinarizer. Apart from putting the Cluster information
// in a binarization queue (areas_), the Area surrounding the left-most and
// right-most shape is searched for possible overlooked components. This
// is done by a series of scanning and extension methods, that look for pixels
// colored similarly to the outermost Shape, and extend the area, in case
// such pixels are found.
// A number of cases have to be handled: If the extension area is much larger
// than the original Cluster itself, the area is discarded, as we probably ran
// into another part of the Image, that is colored similarly by coincidence 
// (such as the frame around a text area).
// We also might run into another Cluster that has been, or will be added to
// the queue for binarization. In such a case, these Areas are merged into 
// a single large area.
class HeliumBinarizer : public Binarizer {
  public:
    // Create a new HeliumBinarizer that will operate on the given Image.
    HeliumBinarizer(const Image& image);
    
    // Add a Cluster to the Binarizer. It will be added to an internal list
    // of Areas, along with a search area around the Cluster, to search for
    // similarly colored components.
    void AddCluster(const Cluster& cluster);
    
    // Convenience method to add multiple Clusters. This simply makes multiple
    // calls to AddCluster(const Cluster&).
    void AddClusters(const ClusterArray& clusters);
    
    // Extracts the next Mask from the Image, along with its bounding box
    // within the image. Note, that these bounds may differ from the original
    // Cluster bounds, as the HeliumBinarizer attempts to extend Cluster
    // bounds, if it finds adjacent components of a similar color.
    // Returns false, if no further Mask could be extracted.
    bool GetNextMask(Mask& out_mask, Box& out_bounds);
    
  private:
    // The Area structure is used internally to mark the areas that require
    // binarization. They not only hold the bounding box information, but also
    // the expected color and color variance, and an ID that specifies which
    // Areas belong to the same Cluster.
    struct Area {
      Box bounds;
      uint8 id;
      Color expected;
      Color variance;
      bool binarized;
      
      // Constructs an Area with all members set to 0.
      Area() : bounds(), id(0), expected(0), variance(0), binarized(0) {
      }
      
      // Constructs an Area with the given bounds, id, expected color and color
      // variance.
      Area(Box area_bounds, uint8 area_id, Color area_exp, Color area_var)
        : bounds(area_bounds), 
          id(area_id), 
          expected(area_exp), 
          variance(area_var),
          binarized(false) {
      }
    };
    
    // The AreaCluster structure is used to hold multiple Boxes, that belong
    // to the same Cluster. This structure is required when scanning for 
    // potential areas around the Cluster, as these are not necessarily 
    // rectangular, but do consist of multiple rectangular squares. 
    struct AreaCluster : ReferenceCounted {
      Box bounds;
      Array<Box>* boxes;
      uint8 id;
      Color expected;
      Color variance;
      
      // Contructs an empty AreaCluster.
      AreaCluster() : 
        ReferenceCounted(),
        bounds(), 
        boxes(NULL), 
        id(0), 
        expected(0), 
        variance(0) {
      }
      
      // Contructs an AreaCluster with the given boxes, the total bounds of
      // these boxes, an ID, and the expected color and color variance.
      AreaCluster(Array<Box>* box_array, Box total_bounds, uint8 cluster_id,
                  Color cluster_exp, Color cluster_var)
        : ReferenceCounted(),
          bounds(total_bounds), boxes(box_array), id(cluster_id),
          expected(cluster_exp), variance(cluster_var) {
      }
        
      // Deallocates the AreaCluster, if it is not referenced anymore.
      ~AreaCluster() {
        if (ShouldDelete()) DeleteData();
      }
      
      // Deletes the Box Array.
      void DeleteData() {
        delete boxes;
        ReferenceCounted::DeleteData(); // Call super
      }
    };

    Array<Area> areas_;
    Array<AreaCluster> extension_clusters_;
    unsigned current_index_;
    uint8 number_of_clusters_;
    Unions cluster_classes_;
    
    // Adds an Area for the specified Shape to the areas_ Array for 
    // binarization.
    void AddShapeArea(const Shape& shape, uint8 id);
    
    // Adds an Area, that lies in between two Shapes, to the areas_ Array for
    // binarization.
    void AddInBetweenArea(const Shape& left, const Shape& right, uint8 id);
     
    // Helper method to check if the specified bounds lie within the source
    // Image.
    inline bool CheckBounds(const Box& bounds) const;
    
    // This method uses the other Extend... methods to search for new 
    // components, that the text detector might have missed. It does this by
    // scanning the surrounding area of the Shape for pixels that are of a
    // similar color as the Shape color. If the detected area is of an 
    // acceptable size, it is added to the binarization queue.
    void ExtendEnd(const Shape& shape, uint8 id, bool right_end);
    
    // Recursively scans for similarly colored pixels to the top of the 
    // specified Area.
    void ExtendTopEndRec(const Area& area, Array<Box>* extensions);
    
    // Recursively scans for similarly colored pixels to the bottom of the 
    // specified Area.
    void ExtendBottomEndRec(const Area& area, Array<Box>* extensions);
    
    // Recursively scans for similarly colored pixels to the left and right of 
    //the specified Area.
    void ExtendEndRec(const Area& area, 
                      Array<Box>* extensions, 
                      bool to_right);
    
    
    // Finds and outputs the Areas that belong to the same class ID in the 
    // given list of Areas. 
    void FindAreasOfClass(int class_id, 
                          Array<Area>& areas, 
                          Array<Area>& found);
    
    // Finds and outputs the Areas that belong to the same class ID in the 
    // given list of AreaClusters. 
    void FindAreasOfClassInClusters(int class_id, 
                                    Array<AreaCluster>& clusters, 
                                    Array<Area>& found);
                          
    // If, due to Area extension, an Area overlaps with another, this method is
    // called to merge the overlapping areas to one.
    void MergeOverlaps(const AreaCluster& cluster);
    
    // This method binarizes the given Area in the input Image into the given
    // allocated Mask. The mask_origin parameter must contain the origin of the
    // given mask in the input image.
    void BinarizeAreaToMask(const Area& area, 
                            Mask& mask, 
                            const Point& mask_origin);

    // Returns the bounding box of the areas specified.
    Box BoundsOfAreas(const Array<Area>& areas);
    
    // Returns true if pixels similarly colored to the specified area were 
    // found to the left of the area.
    bool ScanLeft(const Area& area) const;
    
    // Returns true if pixels similarly colored to the specified area were 
    // found to the top of the area.
    bool ScanTop(const Area& area) const;
    
    // Returns true if pixels similarly colored to the specified area were 
    // found to the right of the area.
    bool ScanRight(const Area& area) const;
    
    // Returns true if pixels similarly colored to the specified area were 
    // found to the bottom of the area.
    bool ScanBottom(const Area& area) const;
    
    // Returns true if pixels similarly colored to the specified area were 
    // found in the middle horizontal line of the area.
    bool ScanHMiddle(const Area& area) const;
    
    // Returns true if pixels similarly colored to the specified area were 
    // found in the middle vertical line of the area.
    bool ScanVMiddle(const Area& area) const;
    
    // Returns true if pixels similarly colored to the specified area were 
    // found on the horizontal line, starting from the given point and 
    // with the same width as the area. This method is used by the above 
    // Scan... methods.
    bool ScanHorizontal(const Area& area, const Point& p) const;
    
    // Returns true if pixels similarly colored to the specified area were 
    // found on the vertical line, starting from the given point and 
    // with the same height as the area. This method is used by the above 
    // Scan... methods.
    bool ScanVertical(const Area& area, const Point& p) const;
};

} // namespace

#endif  // HELIUM_MOBILEBINARIZER_H__
