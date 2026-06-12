// Alpha Stick pod v0: Phase 0 bench rig (OpenSCAD source)
//
// Purpose: validate the V2 sensing + centering + pivot physics
// (docs/DESIGN_V2.md) using the parts on docs/PHASE0_PARTS.md, especially
// the SparkFun Qwiic TMAG5273 breakout (25.4 mm square). This is NOT the
// final 20 x 20 mm pod; it is the rig that produces the numbers the final
// pod will be sized from.
//
// v0 deviations from the production pod described in docs/HARDWARE.md:
//  - Pivot is a printed dome on the hub riding in a PTFE-tape-lined socket,
//    not a discrete steel ball (zero extra parts; the bench compares feel).
//  - Centering/preload comes from the sense magnet attracted to a ring
//    magnet or steel disc in the carrier below the breakout. The steel
//    washer armature returns with the real pod if the bench prefers it.
//  - No detent mechanism on the carrier; at gram-scale loads the printed
//    thread holds position by friction.
//
// Print guidance: docs/PRINTING.md. Hub and housing at 0.12 mm layers.
// Hub prints neck-down (dome up). Housing prints collar-down (socket up).
//
// Select what to render:
//   part = "hub" | "housing" | "base" | "carrier" | "topper" | "guide"
//        | "plate"    (all parts laid out for printing)
//        | "section"  (assembly cross-section for sanity checking)

part = "plate";

// -------------------------------------------------------------------------
// Parameters (mm). The bench exists to tune these; change and re-export.
// -------------------------------------------------------------------------

// sensing stack
pcb_size        = 25.4;   // SparkFun Qwiic breakout, square
pcb_thick       = 1.6;
sensor_top      = 0.9;    // IC top above PCB top face
sensor_gap      = 1.5;    // magnet bottom face to IC top (the design gap)

// sense magnet (N52 D4x2 diametric)
mag_d           = 4.0;
mag_h           = 2.0;
mag_proud       = 0.2;    // how far the magnet sticks out of the dome flat
fit_mag         = 0.2;    // diametral press/glue clearance for magnet pockets

// dome pivot
dome_r          = 6.0;
dome_flat_drop  = 5.0;    // flat cut distance below sphere center
seat_clear      = 0.2;    // socket radius oversize, leaves room for PTFE tape
seat_hole_r     = 4.0;    // socket center opening (magnet sightline)
seat_rim_r      = 5.5;    // outer radius of the spherical contact band

// hub
flange_r        = 8.0;
flange_t        = 2.0;
neck_r          = 2.8;
tube_d          = 2.6;    // 2.5 mm carbon tube + glue
tube_depth      = 8.0;
hub_top         = 16.0;   // hub overall height above base datum (z = 0)
pin_d           = 2.0;    // anti-yaw pins, radial at the flange
pin_reach       = 10.8;   // pin tip radius from axis

// housing
plate_z0        = 5.0;    // underside of the top plate (clears 3 mm Qwiic connectors)
plate_t         = 3.0;
plate_bore_r    = 7.0;    // clearance bore over the dome equator (r 6 + sway)
collar_ir       = 8.5;
collar_or       = 10.5;
collar_top      = 14.0;
slot_w          = 2.6;    // anti-yaw slot width (pin 2.0 + clearance)
slot_bottom     = 8.2;    // sets the tilt limit, ~7 deg with these numbers
housing_sq      = 40.0;   // post footprint, square
post_sq         = 6.0;

// bench base
base_sq         = 44.0;
base_t          = 6.0;
pocket_clear    = 0.4;    // breakout pocket oversize per side total
boss_d          = 18.0;
boss_len        = 10.0;   // threaded boss below the base
bore_plain_d    = 12.5;   // plain travel section above the thread
bore_plain_z    = 6.0;    // plain section depth below pocket floor
screw_pilot_d   = 2.8;    // M3 self-tap pilots (4.0 for heat-set inserts)

// adjuster carrier (M12 x 0.75 per docs/PRINTING.md)
thr_d           = 12.0;
thr_pitch       = 0.75;
thr_depth       = 0.45;
thr_clr         = 0.18;   // radial slop; tune per printer (PRINTING.md)
car_head_h      = 3.0;    // plain unthreaded top section
car_thread_len  = 11.0;   // grip stops on the boss tip just before the head
                          // can touch the breakout underside (0.4 mm margin)
car_pocket_d    = 10.4;   // ring magnet D10 or M5 washer stack, drop-in
car_pocket_h    = 3.0;
grip_d          = 18.0;
grip_h          = 3.0;

// topper and force-test guide
topper_ball_d   = 8.0;
guide_h         = 50.0;   // string guide notch near the 40 mm grip point

$fa = 2;
$fs = 0.35;

// -------------------------------------------------------------------------
// Derived stack (z = 0 is the bench base top face; PCB top sits flush at 0)
// -------------------------------------------------------------------------

mag_face_z   = sensor_top + sensor_gap;            // 2.4: magnet bottom face
dome_flat_z  = mag_face_z + mag_proud;             // 2.6: hub bottom flat
pivot_z      = dome_flat_z + dome_flat_drop;       // 7.6: sphere center
seat_r       = dome_r + seat_clear;                // 6.2: socket sphere
flange_z0    = plate_z0 + plate_t + 0.4;           // 8.4: flange floats over plate
pin_z        = flange_z0 + flange_t / 2;           // 9.4: pin centerline
echo("pivot center z", pivot_z);
echo("tilt limit deg",
     asin((pin_z - slot_bottom) / ((pin_reach + flange_r) / 2)));

module mirror2(v = [1, 0, 0]) { children(); mirror(v) children(); }

// -------------------------------------------------------------------------
// Printed thread: trapezoid tooth swept by a twisted extrude. Tooth angular
// width sets its axial thickness (width_deg / 360 * pitch).
// -------------------------------------------------------------------------

module thread_solid(d_major, pitch, length, depth, root_frac = 0.62,
                    crest_frac = 0.22, radial_off = 0) {
    r_maj = d_major / 2 + radial_off;
    r_min = r_maj - depth;
    root_deg  = 360 * root_frac;
    crest_deg = 360 * crest_frac;
    turns = length / pitch;
    linear_extrude(height = length, twist = -360 * turns, convexity = 10,
                   slices = ceil(turns * 24))
        union() {
            circle(r = r_min, $fn = 72);
            polygon([
                for (a = [-root_deg / 2 : root_deg / 24 : root_deg / 2])
                    [r_min * cos(a) * 0.999, r_min * sin(a) * 0.999],
                for (a = [crest_deg / 2 : -crest_deg / 12 : -crest_deg / 2])
                    [r_maj * cos(a), r_maj * sin(a)]
            ]);
        }
}

module thread_external(length) {
    thread_solid(thr_d, thr_pitch, length, thr_depth, radial_off = -thr_clr);
}

module thread_internal_cut(length) {
    // oversized solid to subtract from a boss
    thread_solid(thr_d, thr_pitch, length, thr_depth, radial_off = thr_clr);
}

// -------------------------------------------------------------------------
// stick-hub-v0: dome pivot, anti-yaw flange, tube neck, magnet pocket
// -------------------------------------------------------------------------

module hub() {
    difference() {
        union() {
            // dome: sphere trimmed to the working band
            intersection() {
                translate([0, 0, pivot_z]) sphere(r = dome_r);
                translate([0, 0, dome_flat_z])
                    cylinder(r = dome_r + 1, h = flange_z0 - dome_flat_z);
            }
            // flange + anti-yaw pins
            translate([0, 0, flange_z0]) cylinder(r = flange_r, h = flange_t);
            mirror2() translate([0, 0, pin_z]) rotate([0, 90, 0])
                cylinder(d = pin_d, h = pin_reach);
            // neck up to the tube socket
            translate([0, 0, flange_z0 + flange_t])
                cylinder(r = neck_r, h = hub_top - flange_z0 - flange_t);
        }
        // magnet pocket in the dome flat
        translate([0, 0, dome_flat_z - mag_proud - 0.01])
            cylinder(d = mag_d + fit_mag, h = mag_h + 0.01);
        // carbon tube bore
        translate([0, 0, hub_top - tube_depth])
            cylinder(d = tube_d, h = tube_depth + 0.01);
    }
}

// -------------------------------------------------------------------------
// seat-housing-v0: socket boss under a bridge plate, anti-yaw collar, posts
// -------------------------------------------------------------------------

module housing() {
    boss_top = plate_z0 + 0.01;
    boss_z0  = 2.5;                      // clears the sensor IC below
    difference() {
        union() {
            // socket boss hanging under the plate center
            translate([0, 0, boss_z0])
                cylinder(r = seat_rim_r + 2.0, h = boss_top - boss_z0);
            // top plate
            translate([0, 0, plate_z0]) linear_extrude(plate_t)
                square(housing_sq, center = true);
            // anti-yaw collar
            translate([0, 0, plate_z0 + plate_t])
                cylinder(r = collar_or, h = collar_top - plate_z0 - plate_t);
            // corner posts down to the base
            for (x = [-1, 1], y = [-1, 1])
                translate([x * (housing_sq - post_sq) / 2,
                           y * (housing_sq - post_sq) / 2, 0])
                    linear_extrude(plate_z0 + 0.01)
                        square(post_sq, center = true);
        }
        // spherical socket (line with PTFE tape) and its center opening
        translate([0, 0, pivot_z]) sphere(r = seat_r);
        translate([0, 0, boss_z0 - 0.01])
            cylinder(r = seat_hole_r, h = pivot_z - boss_z0);
        // clearance bore through the plate over the dome equator
        translate([0, 0, plate_z0 - 0.5])
            cylinder(r = plate_bore_r, h = plate_t + 1);
        // collar inner bore
        translate([0, 0, plate_z0 + plate_t - 0.01])
            cylinder(r = collar_ir, h = collar_top);
        // anti-yaw slots on the X axis (where the pins are), open at the
        // collar top so the hub drops in
        mirror2() translate([collar_ir - 1, -slot_w / 2, slot_bottom])
            cube([collar_or - collar_ir + 2, slot_w,
                  collar_top - slot_bottom + 1]);
        // M3 through-holes in the posts
        for (x = [-1, 1], y = [-1, 1])
            translate([x * (housing_sq - post_sq) / 2,
                       y * (housing_sq - post_sq) / 2, -0.5])
                cylinder(d = 3.4, h = plate_z0 + plate_t + 1);
    }
}

// -------------------------------------------------------------------------
// bench-base-v0: breakout pocket, plain+threaded carrier bore, post pilots
// -------------------------------------------------------------------------

module base() {
    pocket = pcb_size + pocket_clear;
    difference() {
        union() {
            translate([0, 0, -base_t]) linear_extrude(base_t)
                square(base_sq, center = true);
            translate([0, 0, -base_t - boss_len])
                cylinder(d = boss_d, h = boss_len + 0.01);
        }
        // breakout pocket, PCB top lands flush with z = 0
        translate([-pocket / 2, -pocket / 2, -pcb_thick])
            cube([pocket, pocket, pcb_thick + 0.01]);
        // Qwiic cable reliefs on two opposite pocket edges
        mirror2() translate([pocket / 2 - 0.01, -6, -pcb_thick])
            cube([(base_sq - pocket) / 2 + 0.02, 12, pcb_thick + 0.01]);
        // plain travel bore, then the threaded section to the boss tip
        translate([0, 0, -pcb_thick - bore_plain_z])
            cylinder(d = bore_plain_d, h = bore_plain_z + 0.02);
        translate([0, 0, -base_t - boss_len - 0.01])
            thread_internal_cut(base_t + boss_len - pcb_thick - bore_plain_z + 0.02);
        // post pilots (self-tap M3 by default)
        for (x = [-1, 1], y = [-1, 1])
            translate([x * (housing_sq - post_sq) / 2,
                       y * (housing_sq - post_sq) / 2, -base_t - 0.01])
                cylinder(d = screw_pilot_d, h = base_t + 0.02);
        // guide-arm pilots on one edge, 20 mm apart to match the guide feet
        mirror2([0, 1, 0]) translate([base_sq / 2 - 4, 10, -base_t - 0.01])
            cylinder(d = screw_pilot_d, h = base_t + 0.02);
    }
}

// -------------------------------------------------------------------------
// adjuster-carrier-v0: plain head with magnet pocket, thread, grip wheel
// -------------------------------------------------------------------------

module carrier() {
    difference() {
        union() {
            cylinder(d = grip_d, h = grip_h);                       // grip
            translate([0, 0, grip_h]) thread_external(car_thread_len);
            translate([0, 0, grip_h + car_thread_len])
                cylinder(d = thr_d, h = car_head_h);                // plain head
        }
        // magnet / washer-stack pocket in the top face
        translate([0, 0, grip_h + car_thread_len + car_head_h - car_pocket_h])
            cylinder(d = car_pocket_d, h = car_pocket_h + 0.01);
        // poke-out hole
        translate([0, 0, -0.01]) cylinder(d = 3.0, h = grip_h + car_thread_len);
        // grip scallops
        for (a = [0 : 30 : 359]) rotate([0, 0, a])
            translate([grip_d / 2 + 0.4, 0, -0.01])
                cylinder(d = 3.0, h = grip_h + 0.02);
    }
}

// -------------------------------------------------------------------------
// topper-ball-v0 and the force-test string guide
// -------------------------------------------------------------------------

module topper() {
    difference() {
        union() {
            sphere(d = topper_ball_d);
            translate([0, 0, -topper_ball_d / 2 - 1])
                cylinder(d1 = 4.5, d2 = topper_ball_d * 0.8,
                         h = topper_ball_d / 2 + 1);
        }
        translate([0, 0, -topper_ball_d / 2 - 1.01])
            cylinder(d = tube_d - 0.2, h = 6);  // light press fit, glue to taste
    }
}

module guide() {
    difference() {
        union() {
            linear_extrude(3) hull() {
                translate([-10, 0]) circle(d = 8);
                translate([10, 0]) circle(d = 8);
            }
            translate([-4, -4, 0]) cube([8, 8, guide_h]);
        }
        mirror2() translate([10, 0, -0.01]) cylinder(d = 3.4, h = 3.02);
        // rounded string notch near the 40 mm grip height
        translate([0, -5, guide_h - 3]) rotate([-90, 0, 0])
            cylinder(d = 4, h = 10);
    }
}

// -------------------------------------------------------------------------
// Output selection
// -------------------------------------------------------------------------

module plate_layout() {
    translate([-35, 30, 0]) translate([0, 0, hub_top]) rotate([180, 0, 0]) hub();
    translate([30, 30, 0]) translate([0, 0, collar_top]) rotate([180, 0, 0]) housing();
    translate([0, -25, base_t + boss_len]) base();
    translate([-35, -25, 0]) carrier();
    translate([35, -32, topper_ball_d / 2 + 1]) topper();
    translate([35, 8, 0]) rotate([0, 0, 90]) guide();
}

module assembly_section() {
    difference() {
        union() {
            color("tomato") hub();
            color("steelblue") housing();
            color("gray") base();
            // realistic fully-screwed-in pose: head top 0.4 mm under the PCB
            color("gold") translate([0, 0, -(base_t + boss_len + 3)]) carrier();
        }
        translate([0, -60, -60]) cube([120, 120, 120]);  // cut at x = 0
    }
}

if (part == "hub") translate([0, 0, hub_top]) rotate([180, 0, 0]) hub();
else if (part == "housing") translate([0, 0, collar_top]) rotate([180, 0, 0]) housing();
else if (part == "base") translate([0, 0, base_t + boss_len]) base();
else if (part == "carrier") carrier();
else if (part == "topper") translate([0, 0, topper_ball_d / 2 + 1]) topper();
else if (part == "guide") guide();
else if (part == "section") assembly_section();
else plate_layout();
