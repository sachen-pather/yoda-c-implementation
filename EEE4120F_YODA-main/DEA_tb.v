`timescale 1ns/1ps

module DEA_tb;
  // Testbench signals
  reg reset;
  reg dclk;
  reg [7:0] din;
  wire [7:0] dout;
  
  // Internal control signal
  reg kset;
  
  // Instantiate the DEA module
  DEA dut (
    .kset(kset),
    .reset(reset),
    .dclk(dclk),
    .din(din),
    .dout(dout)
  );
  

  
  // Clock generation
  initial begin
    dclk = 0;
    forever #5 dclk = ~dclk; // 100MHz clock (10ns period)
  end
  
  // Test procedure
  initial begin
    // Initialize signals
    reset = 1;
    kset = 0;
    din = 8'h00;
    
    // Reset the system
    #20;
    reset = 0;
    
    // Test case 1: Load 4 keys then encrypt
    $display("Test Case 1: Load 4 keys then encrypt");
    
    // Load keys (kset=1)
    kset = 1;
    din = 8'hAA; #10; // Key 1
    din = 8'hBB; #10; // Key 2
    din = 8'hCC; #10; // Key 3
    din = 8'hDD; #10; // Key 4
    
    // Switch to encryption mode (kset=0)
    kset = 0;
    // Encrypt test data
    din = 8'h12; #10;
    $display("Input: %h, Encrypted: %h (Expected: %h)", din, dout, 8'hAA ^ 8'h12);
    
    din = 8'h34; #10;
    $display("Input: %h, Encrypted: %h (Expected: %h)", din, dout, 8'hBB ^ 8'h34);
    
    din = 8'h56; #10;
    $display("Input: %h, Encrypted: %h (Expected: %h)", din, dout, 8'hCC ^ 8'h56);
    
    din = 8'h78; #10;
    $display("Input: %h, Encrypted: %h (Expected: %h)", din, dout, 8'hDD ^ 8'h78);
    
    // Test case 2: Reset and load 2 keys
    $display("\nTest Case 2: Reset and load 2 keys");
    reset = 1; #10;
    reset = 0; #10;
    
    kset = 1;
    din = 8'h55; #10; // Key 1
    din = 8'h66; #10; // Key 2
    
    kset = 0;
    
    // Encrypt test data - should only use first 2 keys
    din = 8'h11; #10;
    $display("Input: %h, Encrypted: %h (Expected: %h)", din, dout, 8'h55 ^ 8'h11);
    
    din = 8'h22; #10;
    $display("Input: %h, Encrypted: %h (Expected: %h)", din, dout, 8'h66 ^ 8'h22);
    
    din = 8'h33; #10;
    $display("Input: %h, Encrypted: %h (Expected: %h)", din, dout, 8'h55 ^ 8'h33);
    
    // End simulation
    #20;
    $display("Simulation complete");
    $finish;
  end
  
  // Monitor to track internal signals
  initial begin
    $monitor("Time: %t | kset=%b num_keys=%d keys=%h din=%h dout=%h current_key=%h reset=%h dclk=%h",
             $time, kset, dut.k.num_keys, dut.k.keys, din, dout, dut.current_key, reset, dclk);
  end
endmodule