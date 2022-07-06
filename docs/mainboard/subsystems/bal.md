# Cell Balancing
The cell balancing operation is mostly done on the cellboards. However, the mainboard has to compute all the cells that need to be discharged. To synchronize cellboards and the mainboard an FSM is defined:

```diagram
stateDiagram-v2
    direction LR
    [*] --> OFF
    OFF --> CMP
    OFF --> OFF
    CMP --> DSC
    CMP --> OFF
    DSC --> COOL
    DSC --> OFF
    DSC --> DSC
    COOL --> CMP
    COOL--> OFF
    COOL --> COOL
```
### - Off (OFF)
The dormant state of the balancing algorithm.

### - Compute (CMP)
Compute phase. This is where the cell selection algorithm is run. If no cells need to be discharged the FSM returns back to the _Off_ state.
The compute phase can be triggered manually via the CLI.

The algorithm is split in two phases:
#### Imbalance computation
The imbalance is the voltage difference between a given cell and the minimum-voltage cell. In this phase an imbalance array that contains the imbalances of every cell is created. In this case the imbalance is centered around the minimum-voltage plus a threshold, that is the maximum imbalance permitted in the pack.

$imbalance[i] = \max(0, voltages[i] −(min\_voltage + threshold))$

#### Cell selection
The cell selection algorithm is very simple: if the imbalance is greater than 0, the cell is selected for discharge

### - Discharge (DSC)
After compute the mainboard sends the list of cells to be discharged to the cellboards that start the actual discharge for a set amount of time. The discharge time depends on configuration but it's in the range from 30 to 120s.

### - Cooldown (COOL)
Cooldown. A small period (5-10s) where voltages are 
