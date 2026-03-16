# MVP and Project Plan

This is an attempt at implementing a high performance limit order book engine in C++.

 ## Goals

The goals of this project is to learn low-latency systems:

- Data structures
- Memory layout
- Event processing
- Latency measurement

## MVP Features

### Setup Project

- [x ] CMake build
- [ x] Configure compiler flags
- [ ] Add unit testing (GoogleTest)
- [ x] Create basic logging utility
- [ ] Create benchmarking harness


### Core

- [ ] Implement `Order` struct
- [ ] Implement `PriceLevel` structure (FIFO queue of orders)
- [ ] Implement `OrderBook` class
- [ ] Implement `OrderSide` enum (BUY / SELL)

### Order Index

- [ ] Implement `order_id → order` lookup map
- [ ] Support fast order lookup
- [ ] Store pointer/iterator to order location

### Add Order

- [ ] Add BUY limit order
- [ ] Add SELL limit order
- [ ] Insert order into correct price level
- [ ] Maintain FIFO ordering

### Cancel Order

- [ ] Cancel order by order_id
- [ ] Remove order from price level queue
- [ ] Update order index

### Matching Engine

- [ ] Detect crossing orders
- [ ] Match against best price
- [ ] Reduce order quantities
- [ ] Remove filled orders
- [ ] Generate trade event

### Event Processing

- [ ] Implement simple event loop
- [ ] Parse order events
- [ ] Dispatch events to matching engine

### CLI Simulator

- [ ] Read events from file
- [ ] Process event stream
- [ ] Print trades to stdout
- [ ] Print final book state
