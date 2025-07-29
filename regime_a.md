# Documentation for regime_a.json

This file defines a series of regimes and cycles.

## Root Object

The root object contains two main keys: `regimes` and `cycles`.

- `regimes`: An array of regime objects.
- `cycles`: An array of cycle objects.

## `regimes` Array

Each object in the `regimes` array represents a single regime and has the following structure:

- `name` (string): The name of the regime (e.g., "Вакуум").
- `condition` (object): Defines the conditions for the regime.
    - `type` (string): The type of condition. Possible values are `"temp"`, `"time"`, or `"both"`.
    - `temp` (number): The temperature value.
    - `time` (number): The time value in minutes.
- `repeat` (object): Defines if the regime should be repeated independently or as part of a cycle.
    - `count` (number): The number of repetitions. Default is `1`.
- `cycle` (object): Defines the cycle information for the regime.
    - `active` (boolean): `true` if the regime is part of a cycle, `false` otherwise.
    - `id` (number): The ID of the cycle this regime belongs to. Default is `0`.
- `note` (string): A field for a status string.
- `state` (object): Represents the state of the regime.
    - `active` (boolean): `true` if the regime is currently active.
    - `error` (boolean): `true` if an error has occurred.
    - `message` (string): A message associated with the current state.

## `cycles` Array

Each object in the `cycles` array defines a cycle that can be referenced by regimes.

- `id` (number): The unique identifier for the cycle.
- `repeat` (number): The number of times the entire cycle should be repeated. Default is `1`.
