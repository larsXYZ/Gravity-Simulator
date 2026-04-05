import json
import os
import matplotlib.pyplot as plt
import matplotlib
import numpy as np

matplotlib.use("Agg")

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPORTS_DIR = os.path.join(SCRIPT_DIR, "benchmark_reports")
COLORS = ["tab:blue", "tab:orange", "tab:green", "tab:red", "tab:purple",
          "tab:brown", "tab:pink", "tab:gray", "tab:olive", "tab:cyan"]


def load_report(path):
    report_path = os.path.join(path, "results.json")
    with open(report_path) as f:
        data = json.load(f)
    return data["config"], data["results"]


def discover_reports():
    reports = []
    for entry in sorted(os.listdir(REPORTS_DIR)):
        results_file = os.path.join(REPORTS_DIR, entry, "results.json")
        if os.path.isfile(results_file):
            reports.append(os.path.join(REPORTS_DIR, entry))
    return reports


def main():
    report_paths = discover_reports()
    if len(report_paths) < 2:
        print(f"Found {len(report_paths)} report(s) in {REPORTS_DIR}. Need at least 2 to compare.")
        return

    reports = []
    for path in report_paths:
        cfg, res = load_report(path)
        label = cfg["description"]
        reports.append((label, cfg, res))

    print(f"Comparing {len(reports)} reports:")
    for i, (label, _, _) in enumerate(reports):
        print(f"  [{i}] {label}")

    # Use first report's config as reference for axes
    planet_counts = reports[0][1]["planet_counts"]
    particle_counts = reports[0][1]["particle_counts"]
    n = len(reports)

    fig, axes = plt.subplots(2, 2, figsize=(16, 12))
    fig.suptitle("Benchmark Comparison", fontsize=16, fontweight="bold", y=1.0)

    # Legend subtitle listing all reports
    for i, (label, _, _) in enumerate(reports):
        color = COLORS[i % len(COLORS)]
        y_pos = 0.96 - i * 0.02
        fig.text(0.5, y_pos, f"[{i}] {label}", ha="center", fontsize=9, color=color)

    subtitle_height = n * 0.02 + 0.06

    # --- Plot 1: Sim time vs planets (0 particles), one line per report ---
    ax = axes[0, 0]
    x = np.arange(len(planet_counts))
    width = 0.8 / n
    for idx, (label, cfg, res) in enumerate(reports):
        vals = [res.get(f"{p}p_0d", {}).get("sim_ms_avg") or 0 for p in planet_counts]
        offset = (idx - (n - 1) / 2) * width
        ax.bar(x + offset, vals, width, label=f"[{idx}]", color=COLORS[idx % len(COLORS)], alpha=0.8)
    ax.set_xticks(x)
    ax.set_xticklabels(planet_counts)
    ax.set_xlabel("Planet Count")
    ax.set_ylabel("Simulation Time (ms/iteration)")
    ax.set_title("Simulation Time (0 particles)")
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.3, axis="y")
    ax.axhline(y=16.67, color="r", linestyle="--", alpha=0.5)

    # --- Plot 2: Sim time vs planets (max particles) ---
    ax = axes[0, 1]
    max_particles = particle_counts[-1]
    for idx, (label, cfg, res) in enumerate(reports):
        vals = [res.get(f"{p}p_{max_particles}d", {}).get("sim_ms_avg") or 0 for p in planet_counts]
        offset = (idx - (n - 1) / 2) * width
        ax.bar(x + offset, vals, width, label=f"[{idx}]", color=COLORS[idx % len(COLORS)], alpha=0.8)
    ax.set_xticks(x)
    ax.set_xticklabels(planet_counts)
    ax.set_xlabel("Planet Count")
    ax.set_ylabel("Simulation Time (ms/iteration)")
    ax.set_title(f"Simulation Time ({max_particles} particles)")
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.3, axis="y")
    ax.axhline(y=16.67, color="r", linestyle="--", alpha=0.5)

    # --- Plot 3: Speedup heatmap (first vs last) ---
    ax = axes[1, 0]
    _, cfg_first, res_first = reports[0]
    _, cfg_last, res_last = reports[-1]
    speedup = np.zeros((len(particle_counts), len(planet_counts)))
    for i, n_particles in enumerate(particle_counts):
        for j, n_planets in enumerate(planet_counts):
            key = f"{n_planets}p_{n_particles}d"
            a_val = res_first.get(key, {}).get("sim_ms_avg") or 0
            b_val = res_last.get(key, {}).get("sim_ms_avg") or 0
            if b_val > 0 and a_val > 0:
                speedup[i, j] = a_val / b_val
            else:
                speedup[i, j] = 1.0

    im = ax.imshow(speedup, aspect="auto", cmap="RdYlGn",
                   vmin=min(0.5, speedup.min()), vmax=max(2.0, speedup.max()))
    ax.set_xticks(range(len(planet_counts)))
    ax.set_xticklabels(planet_counts)
    ax.set_yticks(range(len(particle_counts)))
    ax.set_yticklabels(particle_counts)
    ax.set_xlabel("Planet Count")
    ax.set_ylabel("Particle Count")
    ax.set_title(f"Speedup ([0] / [{n-1}], >1 = [{n-1}] faster)")
    for i in range(len(particle_counts)):
        for j in range(len(planet_counts)):
            val = speedup[i, j]
            color = "white" if val > (speedup.max() + speedup.min()) / 2 else "black"
            ax.text(j, i, f"{val:.2f}x", ha="center", va="center", fontsize=10, fontweight="bold", color=color)
    fig.colorbar(im, ax=ax, label="Speedup ratio")

    # --- Plot 4: Absolute time saved (first - last) ---
    ax = axes[1, 1]
    diff = np.zeros((len(particle_counts), len(planet_counts)))
    for i, n_particles in enumerate(particle_counts):
        for j, n_planets in enumerate(planet_counts):
            key = f"{n_planets}p_{n_particles}d"
            a_val = res_first.get(key, {}).get("sim_ms_avg") or 0
            b_val = res_last.get(key, {}).get("sim_ms_avg") or 0
            diff[i, j] = a_val - b_val

    max_abs = max(abs(diff.min()), abs(diff.max()), 0.1)
    im = ax.imshow(diff, aspect="auto", cmap="RdYlGn", vmin=-max_abs, vmax=max_abs)
    ax.set_xticks(range(len(planet_counts)))
    ax.set_xticklabels(planet_counts)
    ax.set_yticks(range(len(particle_counts)))
    ax.set_yticklabels(particle_counts)
    ax.set_xlabel("Planet Count")
    ax.set_ylabel("Particle Count")
    ax.set_title(f"Time Saved ([0] - [{n-1}], ms, green = [{n-1}] faster)")
    for i in range(len(particle_counts)):
        for j in range(len(planet_counts)):
            val = diff[i, j]
            color = "white" if abs(val) > max_abs * 0.5 else "black"
            ax.text(j, i, f"{val:+.1f}", ha="center", va="center", fontsize=10, fontweight="bold", color=color)
    fig.colorbar(im, ax=ax, label="ms saved")

    plt.tight_layout(rect=[0, 0, 1, 0.90 - subtitle_height + 0.03])

    output_path = os.path.join(REPORTS_DIR, "comparison.png")
    plt.savefig(output_path, dpi=150)
    plt.close()
    print(f"Comparison saved to {output_path}")


if __name__ == "__main__":
    main()
