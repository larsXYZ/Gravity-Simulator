import subprocess
import re
import argparse
import configparser
import itertools
import matplotlib.pyplot as plt
import matplotlib
import numpy as np
import json
import os
import sys
from datetime import datetime

matplotlib.use("Agg")

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
CONFIG_FILE = "benchmark_config.ini"


def parse_args():
    parser = argparse.ArgumentParser(description="Run gravity simulator benchmarks")
    parser.add_argument("description", help="Description of what this benchmark run is testing")
    return parser.parse_args()


def load_config(path=CONFIG_FILE):
    config = configparser.ConfigParser()
    config.read(os.path.join(SCRIPT_DIR, path))

    bench = config["benchmark"]
    output = config["output"]

    return {
        "executable": bench.get("executable", "build/bin/Release/Benchmark.exe"),
        "iterations": bench.getint("iterations", 25),
        "runs_per_config": bench.getint("runs_per_config", 3),
        "planet_counts": [int(x.strip()) for x in bench.get("planet_counts", "0,10,50,100,500,1000").split(",")],
        "particle_counts": [int(x.strip()) for x in bench.get("particle_counts", "0,100,200,1000").split(",")],
        "output_dir": output.get("output_dir", "benchmark_reports"),
    }


def make_output_dir(cfg):
    timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    run_dir = os.path.join(SCRIPT_DIR, cfg["output_dir"], timestamp)
    os.makedirs(run_dir, exist_ok=True)
    return run_dir


def run_benchmark(executable, n_planets, n_particles, iterations):
    exe_path = os.path.join(SCRIPT_DIR, executable)
    result = subprocess.run(
        [exe_path, "-n", str(n_planets), "-p", str(n_particles), "-i", str(iterations)],
        capture_output=True, text=True, timeout=600,
        cwd=os.path.dirname(exe_path)
    )
    output = result.stdout
    sim_match = re.search(r"Average time per iteration:\s+([\d.]+)\s+ms", output)
    render_match = re.search(r"Average render time per frame:\s+([\d.]+)\s+ms", output)

    sim_ms = float(sim_match.group(1)) if sim_match else None
    render_ms = float(render_match.group(1)) if render_match else None
    return sim_ms, render_ms


def main():
    args = parse_args()
    cfg = load_config()
    cfg["description"] = args.description
    run_dir = make_output_dir(cfg)

    planet_counts = cfg["planet_counts"]
    particle_counts = cfg["particle_counts"]
    runs_per_config = cfg["runs_per_config"]
    iterations = cfg["iterations"]
    executable = cfg["executable"]

    configs = list(itertools.product(planet_counts, particle_counts))
    results = {}

    print(f"Description: {cfg['description']}")
    print(f"Config: {CONFIG_FILE}")
    print(f"  Executable:      {executable}")
    print(f"  Iterations:      {iterations}")
    print(f"  Runs per config: {runs_per_config}")
    print(f"  Planet counts:   {planet_counts}")
    print(f"  Particle counts: {particle_counts}")
    print(f"  Output dir:      {run_dir}")
    print(f"Running {len(configs)} configurations, {runs_per_config} runs each ({len(configs) * runs_per_config} total runs)")
    print("=" * 70)

    for idx, (n_planets, n_particles) in enumerate(configs, 1):
        key = f"{n_planets}p_{n_particles}d"
        sim_times = []
        render_times = []

        for run in range(runs_per_config):
            print(f"[{idx}/{len(configs)}] planets={n_planets:>5}, particles={n_particles:>5}, run {run+1}/{runs_per_config}...", end=" ", flush=True)
            try:
                sim_ms, render_ms = run_benchmark(executable, n_planets, n_particles, iterations)
                sim_times.append(sim_ms)
                render_times.append(render_ms)
                print(f"sim={sim_ms:.2f}ms, render={render_ms:.2f}ms")
            except Exception as e:
                print(f"FAILED: {e}")

        results[key] = {
            "planets": n_planets,
            "particles": n_particles,
            "sim_ms_avg": np.mean(sim_times) if sim_times else None,
            "sim_ms_min": np.min(sim_times) if sim_times else None,
            "sim_ms_max": np.max(sim_times) if sim_times else None,
            "render_ms_avg": np.mean(render_times) if render_times else None,
            "render_ms_min": np.min(render_times) if render_times else None,
            "render_ms_max": np.max(render_times) if render_times else None,
        }

    results_path = os.path.join(run_dir, "results.json")
    report_path = os.path.join(run_dir, "report.png")

    with open(results_path, "w") as f:
        json.dump({"config": cfg, "results": results}, f, indent=2)

    generate_report(cfg, results, report_path)
    print(f"\nDone! Results saved to {run_dir}")


def generate_report(cfg, results, report_path):
    planet_counts = cfg["planet_counts"]
    particle_counts = cfg["particle_counts"]

    fig, axes = plt.subplots(2, 2, figsize=(16, 12))
    fig.suptitle("Gravity Simulator Benchmark Report", fontsize=16, fontweight="bold")

    # Description and config subtitle
    fig.text(0.5, 0.965, cfg["description"], ha="center", fontsize=11, fontstyle="italic")
    config_text = (
        f"iterations={cfg['iterations']}  "
        f"runs={cfg['runs_per_config']}  "
        f"planets={cfg['planet_counts']}  "
        f"particles={cfg['particle_counts']}"
    )
    fig.text(0.5, 0.945, config_text, ha="center", fontsize=9, color="gray")

    # --- Plot 1: Simulation time vs planets (one line per particle count) ---
    ax = axes[0, 0]
    for n_particles in particle_counts:
        x, y, y_min, y_max = [], [], [], []
        for n_planets in planet_counts:
            key = f"{n_planets}p_{n_particles}d"
            r = results[key]
            if r["sim_ms_avg"] is not None:
                x.append(n_planets)
                y.append(r["sim_ms_avg"])
                y_min.append(r["sim_ms_avg"] - r["sim_ms_min"])
                y_max.append(r["sim_ms_max"] - r["sim_ms_avg"])
        ax.errorbar(x, y, yerr=[y_min, y_max], marker="o", capsize=3, label=f"{n_particles} particles")
    ax.set_xlabel("Planet Count")
    ax.set_ylabel("Simulation Time (ms/iteration)")
    ax.set_title("Simulation Time vs Planet Count")
    ax.legend()
    ax.grid(True, alpha=0.3)
    ax.axhline(y=16.67, color="r", linestyle="--", alpha=0.5, label="60 FPS budget")

    # --- Plot 2: Render time vs planets ---
    ax = axes[0, 1]
    for n_particles in particle_counts:
        x, y, y_min, y_max = [], [], [], []
        for n_planets in planet_counts:
            key = f"{n_planets}p_{n_particles}d"
            r = results[key]
            if r["render_ms_avg"] is not None:
                x.append(n_planets)
                y.append(r["render_ms_avg"])
                y_min.append(r["render_ms_avg"] - r["render_ms_min"])
                y_max.append(r["render_ms_max"] - r["render_ms_avg"])
        ax.errorbar(x, y, yerr=[y_min, y_max], marker="s", capsize=3, label=f"{n_particles} particles")
    ax.set_xlabel("Planet Count")
    ax.set_ylabel("Render Time (ms/frame)")
    ax.set_title("Render Time vs Planet Count")
    ax.legend()
    ax.grid(True, alpha=0.3)

    # --- Plot 3: Simulation time vs particles (one line per planet count) ---
    ax = axes[1, 0]
    for n_planets in planet_counts:
        x, y = [], []
        for n_particles in particle_counts:
            key = f"{n_planets}p_{n_particles}d"
            r = results[key]
            if r["sim_ms_avg"] is not None:
                x.append(n_particles)
                y.append(r["sim_ms_avg"])
        ax.plot(x, y, marker="o", label=f"{n_planets} planets")
    ax.set_xlabel("Particle Count")
    ax.set_ylabel("Simulation Time (ms/iteration)")
    ax.set_title("Simulation Time vs Particle Count")
    ax.legend()
    ax.grid(True, alpha=0.3)

    # --- Plot 4: Heatmap of total time (sim + render) ---
    ax = axes[1, 1]
    total_grid = np.zeros((len(particle_counts), len(planet_counts)))
    for i, n_particles in enumerate(particle_counts):
        for j, n_planets in enumerate(planet_counts):
            key = f"{n_planets}p_{n_particles}d"
            r = results[key]
            sim = r["sim_ms_avg"] if r["sim_ms_avg"] is not None else 0
            ren = r["render_ms_avg"] if r["render_ms_avg"] is not None else 0
            total_grid[i, j] = sim + ren

    im = ax.imshow(total_grid, aspect="auto", cmap="YlOrRd")
    ax.set_xticks(range(len(planet_counts)))
    ax.set_xticklabels(planet_counts)
    ax.set_yticks(range(len(particle_counts)))
    ax.set_yticklabels(particle_counts)
    ax.set_xlabel("Planet Count")
    ax.set_ylabel("Particle Count")
    ax.set_title("Total Time Heatmap (ms, sim + render)")
    for i in range(len(particle_counts)):
        for j in range(len(planet_counts)):
            val = total_grid[i, j]
            color = "white" if val > total_grid.max() * 0.6 else "black"
            ax.text(j, i, f"{val:.1f}", ha="center", va="center", fontsize=9, color=color)
    fig.colorbar(im, ax=ax, label="ms")

    plt.tight_layout(rect=[0, 0, 1, 0.93])
    plt.savefig(report_path, dpi=150)
    plt.close()


if __name__ == "__main__":
    main()
