#!/usr/bin/env python3
import matplotlib.pyplot as plt

# Calibration data: trim values and measured drift distances
trim_values = [-0.02, -0.01, 0.0, 0.005, 0.01, 0.012, 0.02]
drift_cm = [0.5, 21, 78, 46, 29, 49, 7]

plt.figure(figsize=(10, 6))
plt.scatter(trim_values, drift_cm, color='blue', s=100, zorder=5)
plt.xlabel('Trim Value', fontsize=14)
plt.ylabel('Drift from Lane Center (cm)', fontsize=14)
plt.title('Duckiebot Calibration: Trim vs. Drift Distance', fontsize=16)
plt.grid(True, linestyle='--', alpha=0.7)

# Highlight the best trim value
best_idx = drift_cm.index(min(drift_cm))
plt.annotate(f'Best: trim={trim_values[best_idx]}',
             xy=(trim_values[best_idx], drift_cm[best_idx]),
             xytext=(trim_values[best_idx] + 0.005, drift_cm[best_idx] + 10),
             arrowprops=dict(arrowstyle='->', color='red'),
             fontsize=12, color='red')

plt.tight_layout()
plt.savefig('calibration_plot.png', dpi=150)
plt.show()

print(f"Best trim value: {trim_values[best_idx]} with drift of {drift_cm[best_idx]} cm")
