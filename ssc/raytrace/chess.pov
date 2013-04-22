// Persistence Of Vision raytracer version 2.0 sample file.

// By Ville Saari
// Copyright (c) 1991 Ferry Island Pixelboys

/*
*
* POV-Ray scene description for chess board.
* 
* Created: 01-Feb-91
* Updated: 02-Mar-91
*
* This scene has 430 primitives in objects and 41 in bounding shapes and
* it takes over 40 hours to render by standard amiga.
*
* If you do some nice modifications or additions to this file, please send 
* me a copy. My Internet address is:
*
*         vsaari@niksula.hut.fi
*/

#include "shapes.inc"
#include "colors.inc"
#include "textures.inc"

camera {
   location <59, 20, -48>
   direction <0, 0, 1>
   up <0, 1, 0>
   right <4/3, 0, 0>
   look_at <0, 0, 1>
}

light_source { <800, 600, -200> colour White }

#declare Pawn = union {
   sphere { <0, 7, 0>, 1.5 }

   sphere { <0, 0, 0>, 1 
      scale <1.2, 0.3, 1.2>
      translate 5.5*y
   }

   intersection {
      plane { y, 5.5 }
      object {
         Hyperboloid_Y
         translate 5*y
         scale <0.5, 1, 0.5>
      }
      plane { -y, -2.5 }
   }

   sphere { <0, 0, 0>, 1 
      scale <2, 0.5, 2>
      translate <0, 2.3, 0>
   }

   intersection {
      sphere { <0, 0, 0>, 2.5 }
      plane { -y, 0 }
   }
}

#declare Rook = union {
   intersection {
      union {
         plane { +x, -0.5 }
         plane { -x, -0.5 }
         plane { y, 9 }
      }

      union {
         plane { +z, -0.5 }
         plane { -z, -0.5 }
         plane { y, 9 }
      }

      plane { y, 10 }
      object { Cylinder_Y scale <2, 1, 2> }
      object { Cylinder_Y scale <1.2, 1, 1.2> inverse }
      plane { -y, -8 }
   }

   intersection {
      plane { y, 8 }
      object { Hyperboloid_Y
         scale <1, 1.5, 1>
         translate 5.401924*y
      }
      plane { -y, -3 }
   }

   sphere { <0, 0, 0>, 1 
      scale <2.5, 0.5, 2.5>
      translate 2.8*y
   }

   intersection {
      sphere { <0, 0, 0>, 3 }
      plane { -y, 0 }
   }
}

#declare Knight = union {
   intersection {
      object { Cylinder_Z
         scale <17.875, 17.875, 1>
         translate <-18.625, 7, 0>
         inverse
      }

      object { Cylinder_Z
         scale <17.875, 17.875, 1>
         translate <18.625, 7, 0>
         inverse
      }

      object { Cylinder_X
         scale <1, 5.1, 5.1>
         translate <0, 11.2, -5>
         inverse
      }

      union {
         plane { y, 0
            rotate 30*x
            translate 9.15*y
         }
         plane { z, 0
            rotate -20*x
            translate 10*y
         }
      }

      union {
         plane { -y, 0
            rotate 30*x
            translate 7.15*y
         }
         plane { y, 0
            rotate 60*x
            translate 7.3*y
         }
      }

      union {
         plane { y, 0
            rotate -45*y
         }
         plane { y, 0
            rotate 45*z
         }
         translate 9*y
      }

      object { Cylinder_Y scale <2, 1, 2> }
      sphere { <0, 7, 0>, 4 }
   }

   sphere { <0, 0, 0>, 1 
      scale <2.5, 0.5, 2.5>
      translate <0, 2.8, 0>
   }

   intersection {
      sphere { <0, 0, 0>, 3 }
      plane { -y, 0 }
   }
}

#declare Bishop = union {
   sphere { <0, 10.8, 0>, 0.4 }

   intersection {
      union {
         plane { -z, -0.25 }
         plane { +z, -0.25 }
         plane { y, 0  }
         rotate 30*x
         translate 8.5*y
      }

      sphere { <0, 0, 0>, 1 
         scale <1.4, 2.1, 1.4>
         translate 8.4*y
      }

      plane { -y, -7 }
   }

   sphere { <0, 0, 0>, 1 
      scale <1.5, 0.4, 1.5>
      translate 7*y
   }

   intersection {
      plane { y, 7 }
      object {
         Hyperboloid_Y
         scale <0.6, 1.4, 0.6>
         translate 7*y
      }
      plane { -y, -3 }
   }

   sphere { <0, 0, 0>, 1 
      scale <2.5, 0.5, 2.5>
      translate 2.8*y
   }

   intersection {
      sphere { <0, 0, 0>, 3 }
      plane { -y, 0 }
   }
}

#declare QueenAndKing = union {
   sphere { <0, 10.5, 0>, 1.5 }

   intersection {
      union {
         sphere { <1.75, 12, 0>, 0.9  rotate 150*y }
         sphere { <1.75, 12, 0>, 0.9  rotate 120*y }
         sphere { <1.75, 12, 0>, 0.9  rotate 90*y }
         sphere { <1.75, 12, 0>, 0.9  rotate 60*y }
         sphere { <1.75, 12, 0>, 0.9  rotate 30*y }
         sphere { <1.75, 12, 0>, 0.9  }
         sphere { <1.75, 12, 0>, 0.9  rotate -30*y }
         sphere { <1.75, 12, 0>, 0.9  rotate -60*y }
         sphere { <1.75, 12, 0>, 0.9  rotate -90*y }
         sphere { <1.75, 12, 0>, 0.9  rotate -120*y }
         sphere { <1.75, 12, 0>, 0.9  rotate -150*y }
         sphere { <1.75, 12, 0>, 0.9  rotate  180*y }
         inverse
      }

      plane { y, 11.5 }

      object { QCone_Y
         scale <1, 3, 1>
         translate 5*y
      }

      plane { -y, -8 }
   }

   sphere { <0, 0, 0>, 1
      scale <1.8, 0.4, 1.8>
      translate 8*y
   }

   intersection {
      plane { y, 8 }
      object { Hyperboloid_Y
         scale <0.7, 1.6, 0.7>
         translate 7*y
      }
      plane { -y, -3 }
   }

   sphere { <0, 0, 0>, 1 
      scale <2.5, 0.5, 2.5>
      translate 2.8*y
   }

   intersection {
      sphere { <0, 0, 0>, 3 }
      plane { <0, -1, 0>, 0 }
   }
}

#declare Queen = union {
   sphere { <0, 12.3, 0>, 0.4 }
   object { QueenAndKing }
}

#declare King = union {
   intersection {
      union {
         intersection {
            plane { y, 13 }
            plane { -y, -12.5 }
         }

         intersection {
            plane { +x, 0.25 }
            plane { -x, 0.25 }
         }
      }

      plane { +z,  0.25 }
      plane { -z,  0.25 }
      plane { +x,  0.75 }
      plane { -x,  0.75 }
      plane { +y,  13.5  }
      plane { -y,  -11.5  }
   }

   object { QueenAndKing }
}

#declare WWood = texture {
   pigment {
      wood
      turbulence 0.1
      colour_map {
         [ 0.0 0.35 colour red 0.7  green 0.4
                    colour red 0.7  green 0.4  ]
         [ 0.35 1.0 colour red 0.95 green 0.62
                    colour red 0.95 green 0.62 ]
      }
      scale <0.6, 1000.0, 0.6>
      translate <200.0, 0.0, 100.0>
   }
   finish {
      specular 1
      roughness 0.02
   }
}

#declare BWood = texture {
   pigment {
      wood
      turbulence 0.1
      colour_map {
         [ 0.0 0.55 colour red 0.45 green 0.25
         colour red 0.45 green 0.25 ]
         [ 0.55 1.0 colour red 0.30 green 0.16
         colour red 0.30 green 0.16 ]
      }
      scale <0.6, 1000.0, 0.6>
      translate <100.0, 0.0, -200.0>
   }
   finish {
      specular 1
      roughness 0.02
   }
}

#declare WPawn = object {
   Pawn

   bounded_by { sphere { <0, 4, 0>, 4.72 } }

   texture {
      WWood
      pigment { quick_color red 0.95 green 0.62 }
   }
}

#declare BPawn = object {
   Pawn

   bounded_by { sphere { <0, 4, 0>, 4.72 } }

   texture {
      BWood
      pigment { quick_color red 0.4 green 0.2 }
   }
}

#declare WRook = object {
   Rook

   bounded_by { sphere { <0, 5, 0>, 5.831 } }

   texture {
      WWood
      pigment { quick_color red 0.95 green 0.62 }
   }
}

#declare BRook = object {
   Rook

   bounded_by { sphere { <0, 5, 0>, 5.831 } }

   texture {
      BWood
      pigment { quick_color red 0.4 green 0.2 }
   }
}

#declare WKnight = object {
   Knight

   bounded_by { sphere { <0, 5, 0>, 5.831 } }

   texture {
      WWood
      pigment { quick_color red 0.95 green 0.62 }
   }
}

#declare BKnight = object {
   Knight
   rotate 180*y

   bounded_by { sphere { <0, 5, 0>, 5.831 } }

   texture { 
      BWood
      pigment { quick_color red 0.4 green 0.2 }
   }
}

#declare WBishop = object {
   Bishop

   bounded_by { sphere { <0, 5.5, 0>, 6.265 } }

   texture {
      WWood
      pigment { quick_color red 0.95 green 0.62 }
   }
}

#declare BBishop = object {
   Bishop
   rotate 180*y

   bounded_by { sphere { <0, 5.5 ,0>, 6.265 } }

   texture {
      BWood
      pigment { quick_color red 0.4 green 0.2 }
   }
}

#declare WQueen = object {
   Queen

   bounded_by {
      intersection {
         sphere { <0, 6, 0>, 6.71 }
         object { Cylinder_Y scale <3, 1, 3> }
      }
   }

   texture {
      WWood
      pigment { quick_color red 0.95 green 0.62 }
   }
}

#declare BQueen = object {
   Queen

   bounded_by {
      intersection {
         sphere { <0, 6, 0>, 6.71 }
         object { Cylinder_Y scale <3, 1, 3> }
      }
   }

   texture {
      BWood
      pigment { quick_color red 0.4 green 0.2 }
   }
}

#declare WKing = object {
   King

   bounded_by {
      intersection {
         sphere { <0, 6.5, 0>, 7.16 }
         object { Cylinder_Y scale <3, 1, 3> }
      }
   }

   texture {
      WWood
      pigment { quick_color red 0.95 green 0.62 }
   }
}

#declare BKing = object {
   King

   bounded_by {
      intersection {
         sphere { <0, 6.5, 0>, 7.16 }
         object { Cylinder_Y scale <3, 1, 3> }
      }
   }

   texture {
      BWood
      pigment { quick_color red 0.4 green 0.2 }
   }
}

/* Sky */
sphere { <0, -39000, 0>, 40000
   inverse

   pigment {
      bozo
      turbulence 0.6
      colour_map {
         [0 0.5 colour red 0.4 green 0.5 blue 1
                colour red 0.4 green 0.5 blue 1.0]
         [0.5 0.7 colour red 0.4 green 0.5 blue 1
                  colour red 1 green 1 blue 1.0]
         [0.7 1 colour red 1 green 1 blue 1
                colour red 0.7 green 0.7 blue 0.7]
      }
      scale 500
      quick_color red 0.4 green 0.5 blue 1
   }
   finish {
      ambient 1
      diffuse 0
   }
}

/* Ground */
plane { y, -80
   pigment { Green }
   finish {
      crand 0.05
      ambient 0.5
      diffuse 0.5
   }
}

#declare Frame = intersection {
   plane { +y, -0.0001 }
   plane { -y, 3 }
   plane { -z, 35 }
   plane { <-1, 0, 1>, 0 }
   plane { < 1, 0, 1>, 0 }
}

union {
   union {
      object {
         union {
            object { Frame }
            object { Frame rotate 180*y }
         }

         pigment {
            wood
            turbulence 0.3
            scale <0.8, 1000, 0.8>
            rotate -88*z
            translate <200, 40, -20>
            quick_color red 0.5 green 0.25
         }
         finish {
            specular 1
            roughness 0.02
         }
      } // object

      object {
         union {
            object { Frame rotate -90*y }
            object { Frame rotate  90*y }
         }

         pigment {
            wood
            turbulence 0.3
            scale <0.8, 1000, 0.8>
            rotate -91*x
            translate <100, 30, 0>
            quick_color red 0.5 green 0.25
         }
         finish {
            specular 1
            roughness 0.02
         }
      } // object

      /* Board */
      intersection {
         plane { +x, 32 }
         plane { -x, 32 }
         plane { +y, 0 }
         plane { -y, 1 }
         plane { +z, 32 }
         plane { -z, 32 }

         texture {
            tiles {
               texture {
                  pigment {
                     marble
                     turbulence 1.0
                     colour_map {
                        [0.0 0.7 colour White
                                 colour White]
                        [0.7 0.9 colour White
                                 colour red 0.8 green 0.8 blue 0.8]
                        [0.9 1.0 colour red 0.8 green 0.8 blue 0.8
                                 colour red 0.5 green 0.5 blue 0.5]
                     }
                     scale <0.6, 1, 0.6>
                     rotate -30*y
                  }
                  finish {
                     specular 1
                     roughness 0.02
                     reflection 0.25
                  }
               } // texture
               tile2
               texture {
                  pigment {
                     granite
                     scale <0.3, 1, 0.3>
                     colour_map {
                        [0 1 colour Black
                             colour red 0.5 green 0.5 blue 0.5]
                     }
                  }
                  finish {
                     specular 1
                     roughness 0.02
                     reflection 0.25
                  }
               }
            } // texture
            scale <8, 1, 8>
         } //texture
      } // intersection

      /* Table */
      union {
         intersection {
            plane { +y, -3 }
            plane { -y,  8 }
            sphere { <0, -5.5, 0>, 55 }
         }

         intersection {
            plane { y, -8 }
            object {
               Hyperboloid_Y
               scale <10, 20, 10>
               translate -20*y
            }
         }

         pigment {
            granite
            scale 6
         }
         finish {
            specular 1
            roughness 0.02
            reflection 0.3
         }
      } // union

      bounded_by { plane { y, 0 } }
   }

   union {
      object { WPawn translate <-28, 0, -20> }
      object { WPawn translate <-20, 0, -20> }
      object { WPawn translate <-12, 0, -20> }
      object { WPawn translate < -4, 0, -20> }
      object { WPawn translate <  4, 0, -20> }
      object { WPawn translate < 12, 0, -20> }
      object { WPawn translate < 20, 0, -20> }
      object { WPawn translate < 28, 0, -20> }

      object { WRook   translate <-28, 0, -28> }
      object { WKnight translate <-20, 0, -28> }
      object { WBishop translate <-12, 0, -28> }
      object { WQueen  translate < -4, 0, -28> }
      object { WKing   translate <  4, 0, -28> }
      object { WBishop translate < 12, 0, -28> }
      object { WKnight translate < 20, 0, -28> }
      object { WRook   translate < 28, 0, -28> }

      bounded_by {
         object {
            Cylinder_X
            scale <1, 9.56, 9.56>
            translate <0, 6.5, -24>
         }
      }
   }

   union {
      object { BPawn translate <-28, 0, 20> }
      object { BPawn translate <-20, 0, 20> }
      object { BPawn translate <-12, 0, 20> }
      object { BPawn translate < -4, 0, 20> }
      object { BPawn translate <  4, 0, 20> }
      object { BPawn translate < 12, 0, 20> }
      object { BPawn translate < 20, 0, 20> }
      object { BPawn translate < 28, 0, 20> }

      object { BRook   translate <-28, 0, 28> }
      object { BKnight translate <-20, 0, 28> }
      object { BBishop translate <-12, 0, 28> }
      object { BQueen  translate < -4, 0, 28> }
      object { BKing   translate <  4, 0, 28> }
      object { BBishop translate < 12, 0, 28> }
      object { BKnight translate < 20, 0, 28> }
      object { BRook   translate < 28, 0, 28> }

      bounded_by {
         object {
            Cylinder_X
            scale <1, 9.56, 9.56>
            translate <0, 6.5, 24>
         }
      }
   }

   bounded_by {
      intersection {
         plane { y, 13.5 }
         sphere { -30*y, 63 }
      }
   }
}
