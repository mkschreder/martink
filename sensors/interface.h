#pragma once

struct imu_xyz {
	double x, y, z;
};

struct imu_interface {
	struct imu_xyz (*read_acceleration)(struct imu_interface *imu); 
	struct imu_xyz (*read_omega)(struct imu_interface *imu);
	struct imu_xyz (*read_heading)(struct imu_interface *imu);
	int16_t				 (*read_altitude)(struct imu_interface *imu);
	int16_t				 (*read_pressure)(struct imu_interface *imu);
	int16_t 			 (*read_temperature)(struct imu_interface *imu); 
};
