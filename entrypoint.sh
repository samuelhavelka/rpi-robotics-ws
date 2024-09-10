#!/bin/bash
# Basic entrypoint for ROS / Colcon Docker containers
 
set -e

echo "Provided arguments: $@"

# source bashrc for autocomplete
# echo "source /opt/ros/iron/setup.bash" >> ~/.bashrc

exec "$@"