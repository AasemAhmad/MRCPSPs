import sys
sys.path.append('../')

import visualization_script_json

def execute_visualization(jobs_json, resources_json):
    visualization_script_json.visualize(jobs_json, resources_json)

if __name__ == "__main__":
    execute_visualization('jobs.json', 'resources.json')
