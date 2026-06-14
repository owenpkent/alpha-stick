// =====================================================================
//  Tetra II  -  parametric spherical flexure joint  (OpenSCAD rebuild)
// ---------------------------------------------------------------------
//  Re-creation of Jelle Rommers' "Tetra 2" remote-centre spherical
//  flexure joint (Thingiverse thing:4841850, CC-BY).
//
//  Principle: three thin triangular flexure blades each lie in a plane
//  that passes through a single point (the remote centre of rotation).
//  Three such planes intersect at that point, so the central hub can
//  only rotate about it -> a spherical joint with a remote pivot.
//
//  This is a clean, editable rebuild (NOT a mesh copy of the original).
//  Tune the parameters below, then render (F6) and export STL.
// =====================================================================

/* [Quality] */
$fn = 72;                 // facet count for cylinders / spheres

/* [Main geometry] */
tri_circumradius  = 64;   // overall size; triangle side = R * sqrt(3) ~= 111 mm
rc_distance       = 50;   // remote-centre (rotation point) distance behind face
blade_t           = 0.7;  // flexure blade thickness (0.7 = original spec)
blade_inner_frac  = 0.78; // how far blades converge toward the remote centre (0..1)
n_blades          = 3;    // number of flexure blades (3 = original)

/* [Outer frame] */
rail_d            = 7;    // diameter of the triangular frame rails

/* [Hub / boss] */
hub_d             = 18;   // boss outer diameter
hub_bore          = 6;    // through-bore diameter (0 = solid)
boss_front        = 14;   // how far the boss stands proud of the front face (+z)
hub_back_extra    = 4;    // extra hub length past the blade roots (toward pivot)

/* [Mounting bracket] */
bracket           = true; // draw the wall-mount foot
br_w              = 44;   // plate width  (along X)
br_h              = 40;   // plate height (along Z)
br_t              = 4;    // plate thickness
br_hole_d         = 4.5;  // bolt hole diameter
br_hole_dx        = 26;   // bolt spacing across width
br_hole_dz        = 24;   // bolt spacing across height
br_neck           = 16;   // neck length from triangle edge out to plate
br_tilt           = 18;   // plate tilt (deg) to match original angled foot

// =====================================================================
//  Helpers
// =====================================================================
function unit(v) = v / norm(v);

// base-triangle vertex i (apex pointing up, lying in the z = 0 plane)
function vtx(i) = let (a = 90 + i * 360 / n_blades)
                  [ tri_circumradius * cos(a), tri_circumradius * sin(a), 0 ];

// remote centre of rotation (tetrahedron apex), on the axis behind the face
AP = [0, 0, -rc_distance];

// rounded bar (capsule) between two points
module bar(p1, p2, d) {
    hull() {
        translate(p1) sphere(d / 2);
        translate(p2) sphere(d / 2);
    }
}

// One flexure blade: a thin trapezoidal sheet whose plane contains AP.
// Outer edge P->Q sits on a frame rail (fixed); inner edge converges
// toward the remote centre and bonds to the hub (moving).
module blade(P, Q) {
    ex = unit(Q - P);                 // in-plane, along the base edge
    nn = unit(cross(Q - P, AP - P));  // sheet normal
    ey = cross(nn, ex);               // in-plane, toward the pivot
    Pp = P + blade_inner_frac * (AP - P);
    Qp = Q + blade_inner_frac * (AP - Q);

    // corners expressed in the blade's own 2D frame (origin at P)
    pts = [ [0,                0              ],
            [ (Q  - P) * ex,  (Q  - P) * ey  ],
            [ (Qp - P) * ex,  (Qp - P) * ey  ],
            [ (Pp - P) * ex,  (Pp - P) * ey  ] ];

    M = [ [ex[0], ey[0], nn[0], P[0]],
          [ex[1], ey[1], nn[1], P[1]],
          [ex[2], ey[2], nn[2], P[2]],
          [0,     0,     0,     1   ] ];

    multmatrix(M)
        translate([0, 0, -blade_t / 2])
            linear_extrude(blade_t)
                polygon(pts);
}

// Central hub / output boss with through-bore.
module hub() {
    z0 = -rc_distance * blade_inner_frac - hub_back_extra;  // back end (near pivot)
    z1 = boss_front;                                        // front end
    difference() {
        translate([0, 0, z0]) cylinder(h = z1 - z0, d = hub_d);
        if (hub_bore > 0)
            translate([0, 0, z0 - 1]) cylinder(h = z1 - z0 + 2, d = hub_bore);
    }
}

// Fixed outer triangular frame.
module frame() {
    for (i = [0 : n_blades - 1])
        bar(vtx(i), vtx((i + 1) % n_blades), rail_d);
}

// Wall-mount foot, attached to one base edge and tilted outward.
module bracket() {
    my = -tri_circumradius * cos(180 / n_blades);  // bottom-edge midpoint, on -Y
    bar([0, my, 0], [0, my - br_neck, 0], rail_d);
    translate([0, my - br_neck, 0])
        rotate([br_tilt, 0, 0])
            translate([0, -br_t / 2, 0])
                difference() {
                    cube([br_w, br_t, br_h], center = true);
                    for (sx = [-1, 1], sz = [-1, 1])
                        translate([sx * br_hole_dx / 2, 0, sz * br_hole_dz / 2])
                            rotate([90, 0, 0])
                                cylinder(h = br_t + 2, d = br_hole_d, center = true);
                }
}

// =====================================================================
//  Assembly
// =====================================================================
module tetra2() {
    union() {
        frame();
        for (i = [0 : n_blades - 1])
            blade(vtx(i), vtx((i + 1) % n_blades));
        hub();
        if (bracket) bracket();
    }
}

tetra2();
