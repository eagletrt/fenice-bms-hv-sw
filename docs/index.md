# High-Voltage Battery Management System

A **battery management system** (BMS) is a collection of operations that ensures the **safety** and **efficiency** of the high voltage battery pack.

## Measurements

One of the main purposes of the BMS is to **constantly measure** the internal state of the battery pack, in particular the cells voltages and temperatures as well as the total pack current output.

Each group of data has to be collected and processed in order to check if the pack is in a critical or safe state.

## Critical errors

Another important part of the BMS is the **critical errors** handling.

If at least one of the acquired data is not within the specification the battery pack is in a critical state that needs to be handled.

In a critical state, to keep the battery in a good operating condition, the pack needs to be **isolated** and no operations should take place between the accumulator and the outside.

## Charge and discharge

While charging, the accumulator needs to be able to stop the charge in case of critical errors, to ensure the pack safety.

To maximize the charge of the battery pack the BMS should implement **balancing**, a technique used to keep each cells voltage values close to each other to ensure a uniform charge between all cells.