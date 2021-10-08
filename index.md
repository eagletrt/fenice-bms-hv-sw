# High-Voltage Battery Management System

Battery management is a collection of operations that ensure the safety and efficiency of the battery pack. A basic battery management system should constantly measure cell temperatures and voltages along with the total pack current output and check that each of those values is within specification. If anomalies are detected, the battery should be disconnected immediately via the AIRs.
A good battery management system is also able to keep the battery in good operating conditions, with the goal of maintaining the expected efficiency of the powertrain.

The need of collecting a lot of data from all over the pack has dictated a scattered architecture for the BMS, that uses multiple data acquisition boards (cellboards) and a single control board (mainboard).

```{toctree}
:caption: 'Mainboard:'
:maxdepth: 2

mainboard/index
```
