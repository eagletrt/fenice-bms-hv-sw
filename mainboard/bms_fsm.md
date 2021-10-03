## BMS FSM
The BMS FSM handles the main logic of the battery management. 

```{tikz}
\begin{tikzpicture}[->, >=stealth', shorten >= 5pt, node  distance = 2.5cm, semithick]
	\node[state, initial]   (idle)                                          {$Idle$};
	\node[state]            (pc)        [right=of idle]                 {$PC$};
	\node[state]            (ts_on)     [right=of pc]      {$TS_{on}$};
	\node[state]            (error)     [above=of pc]             {$Error$};
	
	\path (idle)        edge[loop below] (idle)
			    edge (pc)
			    edge[bend left] (error);
	\path (pc)          edge (ts_on)
			    edge (error)
			    edge[loop below] (pc);
	\path (ts_on)       edge[loop below] (ts_on)
			    edge (error)
			    edge[bend left=40] (idle);
	\path (error)       edge[loop right] (error)
			    edge (idle);
\end{tikzpicture}
	
```

## - Idle
When the TS is off and no fatal errors are present, the BMS is in the Idle state.

## - Pre-Charge
The Pre-Charge procedure is done to turn on the Tractive System. It involves the actuation of the AIRs and the monitoring of the bus voltage.
In the entry phase of the pre-charge state, the negative AIR is closed. This initiates the pre-charge procedure. The bus voltage is periodically confronted with the internal voltage and when they are within 10% of each other the positive AIR is closed, ending the pre-charge procedure. The FSM then transitions to the TS_On state. If the bus voltage doesn't rise fast enough, the pre-charge fails and the FSM goes back to Idle.

## - TS_On
In this state the high-voltage bus external to the battery is powered. This is the state in which the car can run, or the battery can be charged.
When the TS_OFF event occurs, the FSM transitions back to Idle, opening both AIRs at the same time.

## - Error
If a fatal error is active the BMS is in this state. If every fatal error expires, then the BMS returns to Idle and can be turned on again.
