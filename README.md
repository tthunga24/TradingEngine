# C++ Algorithmic Trading Engine
A high-performance, multi-threaded, event-driven trading engine written in C++. This engine is designed for low-latency communication with trading strategies written in both C++ and Python, connecting to live brokers through a modular gateway system.

## Key Features
- **Multi-Threaded & Event-Driven:** The engine utilizes a non-blocking event loop that processes market data and order flow concurrently across multiple threads for maximum performance and to avoid locking.

- **Low-Latency:** Uses ZeroMQ (ZMQ) for inter-process communication, allowing for extremely fast communication with the strategy client.

- **Language Agnostic:** The engine is language-agnostic, and the clear API contract allows strategies to be written in any language.

- **Pluggable Broker Gateways:** A clean interface allows for easy extension to different brokers. Currently supports:

  - Interactive Brokers (IBKR): Live and paper trading.

  - Mock Mode: For testing strategies offline using CSV data.
  - More brokers to be implemented/can be easily implemented due to the modularity and provided interface classes.

- **Risk Management:** Core logic to validate orders against market hours and max position sizes, preventing faulty orders. More custom risk parameters can be easily added.

- **High-Performance Logging:** Spdlog logging is enabled by default to asynchronously log and store all engine events. 


## Using the Engine
Communication with the engine is handled over two primary ZMQ channels using a standardized API contract. Strategy clients do not link directly to the engine; they are separate processes that connect to these IPC endpoints.

### 1. Data Channel (Engine → Strategy):
The engine publishes a continuous stream of market data and execution updates on this channel.

  Endpoint: tcp://localhost:5555

  Socket Type: PUB (Engine) → SUB (Client)

  Format: ZMQ Multi-part message: [TOPIC] [JSON_PAYLOAD]

  Data Topics & Payloads
  Tick Data:

  Topic: TICK.<SYMBOL> (e.g., TICK.AAPL)

  Payload: {"timestamp": "...", "data": {"symbol": "AAPL", "price": 150.25, "size": 100}}

  Execution Reports:

  Topic: EXECUTION.<ORDER_ID> (e.g., EXECUTION.1)

  Payload: {"timestamp": "...", "data": {"order_id": 1, "status": "FILLED", "fill_quantity": 50, "avg_fill_price": 150.26}}

  A client must SUBscribe to the specific topics it is interested in (e.g., TICK.SPY).

### 2. Control Channel (Strategy → Engine):
  Strategies send commands (like placing orders or requesting data) to the engine on this channel.

  Endpoint: tcp://localhost:5556

  Socket Type: PUB (Client) → SUB (Engine)

  Format: ZMQ Multi-part message: [COMMAND_TOPIC] [JSON_PAYLOAD]

  Command Topics & Payloads
  Request Market Data:

  Topic: SUBSCRIBE

  Payload: {"topic": "TICK.TSLA"}

  Place a New Order:

  Topic: CREATE_ORDER

  Payload:

  {
    "correlation_id": "strategy_generated_uuid_123",
    "payload": {
    "symbol": "TSLA",
    "side": "BUY",
    "order_type": "LIMIT",
    "quantity": 25,
    "limit_price": 200.50
  }
}

## Building and Running
### Dependencies:
- A modern C++ compiler (C++17)
- CMake (version 3.15+)
- ZeroMQ
- Intel Decimal Floating-Point Math Library

- The other C++ dependencies are automatically handled by CMake.

### Build Instructions:
- Clone the repository.

- Create a build directory: mkdir build && cd build

- Run CMake: cmake ..

- Compile the engine: make

### Running the Engine:
- Configure: Edit the config/config.json file to set your desired mode ("mock" or "paper") and other parameters.

- Run: From the project's root directory, execute the engine: ./build/engine

- Exit: Press Ctrl+C to shutdown.
