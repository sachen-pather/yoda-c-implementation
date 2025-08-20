module key_reg (
  input wire [7:0] din,       //Incoming input data
  input wire reset,           //Reset signal
  input wire dclk,            //Clock Signal
  input wire kset,            //Key set signal
  output reg [3:0]num_keys,   //Total stored keys
  output reg [31:0] keys      //Register to hold all stored keys
);
  //On dclk run
  always@(posedge dclk) begin
    //If reset is high, reset the keys and num_keys
    if (reset) begin 
      keys <= 32'b0;
      num_keys <= 3'b0;
    end

    // If kset is high, store the incoming data into the keys register 
    if (kset == 1) begin  
      //If less than 4 keys are stored, store the incoming data
      //it its corresponding index, and increment the counter
      if (num_keys == 0) begin
        keys[7:0] <= din;
        num_keys <= num_keys + 1;
      end
      
      if (num_keys ==1) begin
        keys[15:8] <= din;
        num_keys <= num_keys+1;
      end

      if (num_keys ==2) begin
        keys[23:16] <= din;
        num_keys <= num_keys+1;
      end
  
      if (num_keys ==3) begin
        keys[31:24] <= din;
        num_keys <= num_keys+1;
      end
  end
  end
endmodule



