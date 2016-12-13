import sys
import numpy as np

import gpof
from gpof.strategies import GradientDescentConfig
from gpof.strategies import Runner
from gpof.runset import RunSet
from gpof.strategies import RV

# For each given FAST9 detection, look for the pareto front for the two objectives :
# computational time and percentage of errors > 3px

# The detector is FAST n = 9 used with the blockwise selection strategy.

detector_configurations=[
    [5, 3],
    [7, 3],
    [10, 3],
    [10, 5],
    [15, 6],
    [20, 7],
    [30, 10]]

# Global score: average for all detector configuration, the area under the pareto front.

# Starting point of the gradient descent.
configuration={
    'nscales': 3,
    'winsize': 9,
    'propagation': 4,
    'min_scale': 0,
    'patchsize': 5, 
    # fixed in the experiments
    'detector_th': 10,
    'block_size': 10
}

parameter_space = {
    'nscales': np.arange(1,6, 1),
    'winsize': np.arange(5, 15, 2),
    'propagation': np.arange(0, 10, 1),
    'patchsize': np.arange(3, 10, 1)
}

pareto_runs=gpof.open_runset("pareto.rs")

runner=gpof.command_runner(("{} %config_file %result_file").format(' '.join(sys.argv[1:])))

for detector_config in detector_configurations:
    for alpha in np.arange(0, 1, 0.02):
        
        cost = lambda r: alpha * r['errors'] + (1 - alpha) * r['runtime'] / (1.0 * 1000)
        configuration['detector_th'] = detector_config[0];
        configuration['block_size'] = detector_config[1];

        gd_conf=GradientDescentConfig(
            max_iterations = 10,
            starting_point = configuration,
            parameters = parameter_space,
            iterator = 'linear_prediction',
            cost = cost)

        gpof.strategies.gradient_descent2(runner, gd_conf)
        run = gpof.runset.select_best_run(runner.runset, cost)
        pareto_runs.record(run)

# Here, pareto.rs and pareto_runs contains all the best runsets
# for each detector_config / alpha couple.
