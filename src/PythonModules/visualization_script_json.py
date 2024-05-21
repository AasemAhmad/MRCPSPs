import matplotlib.pyplot as plt
import numpy as np
import matplotlib.colors as mcolors
import json

seed = 45
rng = np.random.default_rng(seed)

class JobAllocation:
    def __init__(self, job_id, start_time, duration, mode_id, units_map, resource_ids):
        self.job_id = job_id
        self.start_time = start_time
        self.duration = duration
        self.mode_id = mode_id
        self.units_map = units_map
        self.resource_ids = resource_ids

class Resource:
    def __init__(self, id, units):
        self.id = id
        self.units = units

def read_job_allocations_from_json(json_filename):
    with open(json_filename, 'r') as json_file:
        data = json.load(json_file)
        job_allocations = []
        for job_data in data["jobs"]:
            job_allocations.append(JobAllocation(
                job_data["job_id"],
                job_data["start_time"],
                job_data["duration"],
                job_data["mode_id"],
                job_data["units_map"],
                job_data["resource_ids"]
            ))
    return job_allocations

def read_resources_from_json(json_filename):
    with open(json_filename, 'r') as json_file:
        data = json.load(json_file)
        resources = [Resource(resource_data["id"], resource_data["units"]) for resource_data in data["resources"]]
    return resources

def visualize(jobs_json, resources_json):
    job_allocations_data = read_job_allocations_from_json(jobs_json)
    resources_data  = read_resources_from_json(resources_json)
    num_resources = len(resources_data)
    colors = rng.random((len(job_allocations_data), 3))
    _, axs = plt.subplots(num_resources, 1, sharex=True, figsize=(8, 4 * num_resources))
    
    for i, res in enumerate(resources_data):
        ax = axs[i]
        ax.set_yticks(range(res.units + 1))
        ax.set_ylabel(f'Resource {i+1}')
        ax.grid(True)

        for job in job_allocations_data:
            start_time = job.start_time
            bar_width = job.duration
            bar_middle = start_time + bar_width / 2
            assigned_color = colors[int(job.job_id)]   
            for i, resource_units in enumerate(job.units_map):
                ax = axs[job.resource_ids[i]]
                for unit in resource_units:
                    ax.barh(y=unit, width=bar_width, left=start_time, height=0.5, align='center', color=assigned_color)
                    ax.text(bar_middle, unit, str(job.job_id), ha='center', va='center', color='white' if mcolors.rgb_to_hsv(assigned_color)[2] < 0.5 else 'black')

    plt.suptitle('Gantt Chart', y=1)
    axs[-1].set_xlabel('Time')
    plt.tight_layout()
    plt.show()
