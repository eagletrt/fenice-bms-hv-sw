/**
 * @file		bal.c
 * @brief		This file contains the balancing functions
 *
 * @date		Oct 28, 2019
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author		Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#include "bal.h"

#include "mainboard_config.h"

#include <stddef.h>
#include <string.h>

/**
 * @returns	The index of the maximum value of data
 */
uint8_t _max_index(uint16_t data[], size_t count) {
    uint8_t max = 0;
    for (uint8_t i = 0; i < count; i++) {
        if (data[i] > data[max])
            max = i;
    }

    return max;
}

/**
 * @returns	The index of the minimum value of data
 */
uint16_t _min_index(voltage_t data[], size_t count) {
    uint16_t min_value_index = 0;
    for (uint16_t i = 0; i < count; i++) {
        if (data[i] < data[min_value_index] && data[i] > 0)
            min_value_index = i;
    }

    return min_value_index;
}

/**
 * @brief Reconstructs the solution given the dynamic programming array
 * 
 * @param	DP	dynamic programming work array
 * @param	i	length of DP
 * @param	out	output array
 * @param	out_index	length of output (initialize to 0 please)
 */
void _bal_hateville_solution(uint16_t DP[], uint16_t i, bms_balancing_converted_t cells[], uint16_t *out_index) {
    if (i == 0) {
        return;
    } else if (i == 1) {
        if (DP[1] > 0) {
            // cells[0] |= (1 << 0);
            cells[0].cells_cell0 = 1;
            ++(*out_index);
        }
        return;
    } else if (DP[i] == DP[i - 1]) {
        _bal_hateville_solution(DP, i - 1, cells, out_index);
        return;
    } else {
        _bal_hateville_solution(DP, i - 2, cells, out_index);
        size_t index = (i - 1) / LTC6813_CELL_COUNT;

        switch ((i - 1) % LTC6813_CELL_COUNT)
        {
        case 0:
            cells[index].cells_cell0 = 1;
            break;
        case 1:
            cells[index].cells_cell1 = 1;
            break;
        case 2:
            cells[index].cells_cell2 = 1;
            break;
        case 3:
            cells[index].cells_cell3 = 1;
            break;
        case 4:
            cells[index].cells_cell4 = 1;
            break;
        case 5:
            cells[index].cells_cell5 = 1;
            break;
        case 6:
            cells[index].cells_cell6 = 1;
            break;
        case 7:
            cells[index].cells_cell7 = 1;
            break;
        case 8:
            cells[index].cells_cell8 = 1;
            break;
        case 9:
            cells[index].cells_cell9 = 1;
            break;
        case 10:
            cells[index].cells_cell10 = 1;
            break;
        case 11:
            cells[index].cells_cell11 = 1;
            break;
        case 12:
            cells[index].cells_cell12 = 1;
            break;
        case 13:
            cells[index].cells_cell13 = 1;
            break;
        case 14:
            cells[index].cells_cell14 = 1;
            break;
        case 15:
            cells[index].cells_cell15 = 1;
            break;
        case 16:
            cells[index].cells_cell16 = 1;
            break;
        case 17:
            cells[index].cells_cell17 = 1;
            break;
        }
        ++(*out_index);

        return;
    }
}

/**
 * @brief Hateville problem solver with Dynamic Programming (https://disi.unitn.it/~montreso/asd/handouts/13-pd1.pdf#Outline0.3)
 * 
 * @details	Explanation of this algorithm by the one and only Alberto Montresor ❤️ (https://youtu.be/rrQ300wySmc)
 * 
 * @param	D			Input data
 * @param	count		Input size
 * @param	solution	Output data
 * 
 * @returns	length of the solution array
 */
uint16_t _bal_hateville(uint16_t D[], uint16_t count, bms_balancing_converted_t solution[]) {
    uint16_t DP[PACK_CELL_COUNT + 1];

    DP[0] = 0;
    DP[1] = D[0];

    for (uint16_t i = 2; i < count + 1; i++) {
        DP[i] = MAX(DP[i - 1], DP[i - 2] + D[i - 1]);
    }

    uint16_t out_index = 0;
    _bal_hateville_solution(DP, count, solution, &out_index);
    return out_index;
}

/* @section Public functions */

uint16_t bal_get_cells_to_discharge(
    voltage_t volts[],
    uint16_t volts_count,
    voltage_t threshold,
    bms_balancing_converted_t cells[],
    uint16_t cells_count,
    voltage_t target) {
    uint16_t len = 0;
    uint16_t min_volt;

    if (target == 0)
        min_volt = volts[_min_index(volts, volts_count)];
    else
        min_volt = target;

    memset(cells, 0, sizeof(bms_balancing_converted_t) * cells_count);

    for (uint16_t i = 0; i < volts_count; i++) {
        if (MAX(0, (int32_t)volts[i] - (min_volt + threshold))) {
            size_t index = i / LTC6813_CELL_COUNT;
            switch (i % LTC6813_CELL_COUNT)
            {
            case 0:
                cells[index].cells_cell0 = 1;
                break;
            case 1:
                cells[index].cells_cell1 = 1;
                break;
            case 2:
                cells[index].cells_cell2 = 1;
                break;
            case 3:
                cells[index].cells_cell3 = 1;
                break;
            case 4:
                cells[index].cells_cell4 = 1;
                break;
            case 5:
                cells[index].cells_cell5 = 1;
                break;
            case 6:
                cells[index].cells_cell6 = 1;
                break;
            case 7:
                cells[index].cells_cell7 = 1;
                break;
            case 8:
                cells[index].cells_cell8 = 1;
                break;
            case 9:
                cells[index].cells_cell9 = 1;
                break;
            case 10:
                cells[index].cells_cell10 = 1;
                break;
            case 11:
                cells[index].cells_cell11 = 1;
                break;
            case 12:
                cells[index].cells_cell12 = 1;
                break;
            case 13:
                cells[index].cells_cell13 = 1;
                break;
            case 14:
                cells[index].cells_cell14 = 1;
                break;
            case 15:
                cells[index].cells_cell15 = 1;
                break;
            case 16:
                cells[index].cells_cell16 = 1;
                break;
            case 17:
                cells[index].cells_cell17 = 1;
                break;
            }
            ++len;
        }
    }

    return len;

    /*
	if(target == 0)
		len = bal_compute_imbalance(volts, volts_count, threshold, imbalance);
	else
		len = bal_compute_imbalance_with_target(volts, volts_count, threshold, imbalance, target);

	if (len == 0) {
		return false;
	}

	return bal_exclude_neighbors(imbalance, volts_count, cells);
*/
}

uint16_t bal_compute_imbalance_with_target(
    voltage_t volts[],
    uint16_t count,
    voltage_t threshold,
    uint16_t cells[],
    voltage_t target) {
    uint16_t indexes = 0;

    for (uint16_t i = 0; i < count; i++) {
        cells[i] = max(0, (int32_t)volts[i] - (target + threshold));
        if (cells[i] > 0) {
            indexes++;
        }
    }
    return indexes;
}

uint16_t bal_compute_imbalance(voltage_t volts[], uint16_t count, voltage_t threshold, uint16_t cells[]) {
    uint16_t indexes   = 0;
    uint16_t min_index = _min_index(volts, count);

    for (uint16_t i = 0; i < count; i++) {
        cells[i] = max(0, volts[i] - (volts[min_index] + threshold));
        if (cells[i] > 0) {
            indexes++;
        }
    }
    return indexes;
}

uint16_t bal_exclude_neighbors(uint16_t data[], uint16_t count, bms_balancing_converted_t cells[]) {
    return _bal_hateville(data, count, cells);
}