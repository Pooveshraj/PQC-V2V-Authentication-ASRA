#!/usr/bin/env python3
import sys
import xml.etree.ElementTree as ET


def rebase_route_file(input_path, output_path, window_start=28800.0, window_end=32400.0):
    tree = ET.parse(input_path)
    root = tree.getroot()
    kept = 0
    removed = 0
    for vehicle in list(root):
        if vehicle.tag != "vehicle":
            continue
        depart_str = vehicle.get("depart")
        if depart_str is None:
            continue
        depart = float(depart_str)
        if window_start <= depart < window_end:
            new_depart = depart - window_start
            vehicle.set("depart", f"{new_depart:.2f}")
            kept += 1
        else:
            root.remove(vehicle)
            removed += 1
    tree.write(output_path, xml_declaration=True, encoding="UTF-8")
    print(f"{input_path} -> {output_path}: kept {kept}, removed {removed}")


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 rebase_routes.py <input.rou.xml> <output.rou.xml> [window_start] [window_end]")
        sys.exit(1)
    input_path = sys.argv[1]
    output_path = sys.argv[2]
    window_start = float(sys.argv[3]) if len(sys.argv) > 3 else 28800.0
    window_end = float(sys.argv[4]) if len(sys.argv) > 4 else 32400.0
    rebase_route_file(input_path, output_path, window_start, window_end)
