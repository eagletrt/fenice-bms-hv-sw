# Current
Current measurements involves 2 different sensors, the shunt resistor, measured through the isolated ADC and the 2-channel analog Hall sensor, sampled through internal ADCs.

### Current zeroing
All the 3 measures involve offsets, which are calculated in those situations where the current is 0 (when both AIRs and precharge relay are open).

### Shunt resistor
The value coming from the ADC is converted with this formula: `(volt - shunt_offset) / (1e-4f * 500)`

### Hall Sensor
#### High channel
The analog signal is read by adc2 in DMA mode in a 128 items array. When the current need to be retrieved, those values are averaged. The value is converted with this formula: `(499.f / 300.f / 6.67e-3f) * (volt - high_channel_offset)`

#### Low channel
The analog signal is read by adc3 in DMA mode in a 128 items array. When the current need to be retrieved, those values are averaged. The value is converted with this formula: `(499.f / 300.f / 40e-3f) * (volt - low_channel_offset)`