# Main State Machine
The BMS fsm handles the main state machine of the board. It is responsible for the management of the tractive system activation and deactivation.

# States
```diagram
stateDiagram-v2
    direction LR
    [*] --> Idle
    Idle --> AirnClose
    Idle --> Error
    AirnClose --> Idle
    AirnClose --> AirnStatus
    AirnClose --> Error
    AirnStatus --> Precharge
    AirnStatus --> Idle
    AirnStatus --> Error
    Precharge --> On
    Precharge --> Idle
    Precharge --> Error
    On --> Idle
    On --> Error
    Error --> Idle
```

## - **Idle**
When the TS is off and no fatal errors are present, the BMS is in the Idle state.

## - **AirnClose**
When the procedure of activation of the Tractive System is requested. Additional feedbacks are checked in order to close the AIR negative.

## - **AirnStatus**
Checks if the activation of the AIR negative is ok.

## - **Precharge**
The Precharge procedure is done to turn on the Tractive System. It involves the actuation of the precharge relay and the monitoring of the bus voltage.
The bus voltage is periodically confronted with the internal voltage and when they are within 7% of each other the positive AIR is closed and the precharge relay is opened, ending the precharge procedure. The FSM then transitions to the TS_On state. If the bus voltage doesn't rise fast enough, the precharge fails and the FSM goes back to Idle.

## - **TS_On**
In this state the high-voltage bus external to the battery is powered. This is the state in which the car can run, or the battery can be charged.
When the TS_OFF event occurs, the FSM transitions back to Idle, opening both AIRs at the same time.

## - **Error**
If a fatal error is active the BMS is in this state. TS activation requests are ignored. If every fatal error expires, then the BMS returns to Idle and can accept TS on commands again.
