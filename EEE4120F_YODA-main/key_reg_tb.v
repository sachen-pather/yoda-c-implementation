
module key_reg_tb;

  // Inputs
  reg [7:0] din;
  reg reset;
  reg dclk;
  reg kset;

  // Outputs
  wire [2:0] num_keys;
  wire [31:0] keys;

  // DUT
  key_reg uut (
    .din(din),
    .reset(reset),
    .dclk(dclk),
    .kset(kset),
    .num_keys(num_keys),
    .keys(keys)
  );

  initial begin
    // Initial values
    $monitor("Time=%0t | num_keys=%b (%0d) | keys=0x%08b", $time, num_keys, keys);
    dclk = 0;
    reset = 0;
    kset = 0;
    din = 8'b0;

    #10 dclk = 1;

    reset = 1;  // Assert reset
    
    #10 dclk = 0;
    #10 dclk = 1;


    reset = 0;  // Deassert reset
    kset = 1;   //Set kset to latch the key
    din = 8'b10;

    //write the first key
    #10 dclk = 0;
    #10 dclk = 1;

    // Write the second key
    din = 8'b110;

    #10 dclk = 0;
    #10 dclk = 1;

    // Write the third key
    din = 8'b1110;

    #10 dclk = 0;
    #10 dclk = 1;

    // Write the fourth key
    din = 8'b11110;

    #10 dclk = 0;
    #10 dclk = 1;

    kset = 0;  //Deassert the key set line 

    #10 dclk = 0;
    #10 dclk = 1;

    kset = 1;  // Set kset to latch the key
    din = 8'b111110;
    
    #10 dclk = 0;
    #10 dclk = 1;

    din = 8'b111110;
    

    #10 dclk = 0;
    #10 dclk = 1;

    din = 8'b111110;
    
    #10 dclk = 0;
    #10 dclk = 1;

    $stop;
  end

endmodule
