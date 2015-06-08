/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK GPP.
 *
 * REDHAWK GPP is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK GPP is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#include "SystemMonitorReporting.h"
#include "../statistics/CpuUsageAccumulator.h"
#include "../GPP.h"

#include <sstream>

#include <sys/sysinfo.h>

static const size_t BYTES_PER_MEGABYTE = 1024*1024;

SystemMonitorReporting::SystemMonitorReporting( const boost::shared_ptr<const CpuUsageAccumulator>& cpu_usage_accumulator,
                     system_monitor_struct& reporting_data):
cpu_usage_accumulator_(cpu_usage_accumulator),
reporting_data_(reporting_data)
{
    
}

void
SystemMonitorReporting::report()
{
    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open()) {
        bool done = false;
        std::string line;
        float total_memory = 0;
        float committed_memory = 0;
        while ((not done) and (getline(meminfo, line))) {
            if (line.find("CommitLimit:") != std::string::npos) {
                std::istringstream iss(line);
                std::string sub;
                iss>>sub;
                iss>>total_memory;
            }
            if (line.find("Committed_AS:") != std::string::npos) {
                std::istringstream iss(line);
                std::string sub;
                iss>>sub;
                iss>>committed_memory;
            }
        }
        reporting_data_.physical_memory_free = (total_memory - committed_memory)/1024;
    } else {
        struct sysinfo info;
        sysinfo(&info);
        reporting_data_.physical_memory_free = info.freeram / BYTES_PER_MEGABYTE * info.mem_unit;
    }

	//reporting_data_.virtual_memory_total = (info.totalram+info.totalswap) / BYTES_PER_MEGABYTE * info.mem_unit;
	//reporting_data_.virtual_memory_free = (info.freeram+info.freeswap) / BYTES_PER_MEGABYTE * info.mem_unit;
	//reporting_data_.virtual_memory_used = reporting_data_.virtual_memory_total-reporting_data_.virtual_memory_free;
	//reporting_data_.virtual_memory_percent = (double)reporting_data_.virtual_memory_used / (double)reporting_data_.virtual_memory_total * 100.;
	//reporting_data_.physical_memory_total = info.totalram / BYTES_PER_MEGABYTE * info.mem_unit;
	//reporting_data_.physical_memory_free = info.freeram / BYTES_PER_MEGABYTE * info.mem_unit;
	//reporting_data_.physical_memory_used = reporting_data_.physical_memory_total-reporting_data_.physical_memory_free;
	//reporting_data_.physical_memory_percent = (double)reporting_data_.physical_memory_used / (double)reporting_data_.physical_memory_total * 100.;
	//reporting_data_.user_cpu_percent = cpu_usage_accumulator_->get_user_percent();
	//reporting_data_.system_cpu_percent = cpu_usage_accumulator_->get_system_percent();
	reporting_data_.idle_cpu_percent = cpu_usage_accumulator_->get_idle_percent();
	//reporting_data_.cpu_percent = 100.0 - reporting_data_.idle_cpu_percent;
	//reporting_data_.up_time = info.uptime;
	//reporting_data_.up_time_string = format_up_time(reporting_data_.up_time);
	//reporting_data_.last_update_time = time(NULL);
}

std::string
SystemMonitorReporting::format_up_time(unsigned long secondsUp) const
{
	std::stringstream formattedUptime;
	int days;
	int hours;
	int minutes;
	int seconds;

	int leftover;

	days = (int) secondsUp / (60 * 60 * 24);
	leftover = (int) secondsUp - (days * (60 * 60 * 24) );
	hours = (int) leftover / (60 * 60);
	leftover = leftover - (hours * (60 * 60) );
	minutes = (int) leftover / 60;
	seconds = leftover - (minutes * 60);

	formattedUptime << days << "d " << hours << "h " << minutes << "m " << seconds << "s";

	return formattedUptime.str();
}
