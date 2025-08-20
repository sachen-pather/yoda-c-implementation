
module DEA(
  input wire reset,     // Reset signal
  input wire dclk,      // Clock signal
  input wire kset,      // Key set signal
  input wire [7:0] din, // Input data 
  output reg [7:0] dout // Output data

);
  
  // Internal register to store the current key number
  reg [2:0] current_key;
  // Wires to connect to the key register sub-module
  wire [3:0]num_keys;
  wire [31:0] keys;

  //Initialise the key register sub-module and the wire the signals
  key_reg k (.reset(reset), .dclk(dclk), .kset(kset), .din(din), .num_keys(num_keys),
  .keys(keys));

  always @(posedge dclk) begin
     //Reset current key
    if (reset) begin
      dout <= 8'b0;  //Reset output
      current_key <= 2'b0;    
    end

    else if(!kset && num_keys!= 0) begin
      case(current_key)
        3'b00: dout <= keys[7:0] ^ din;       // 1 key
        3'b01: dout <= keys[15:8] ^ din;      // 2 keys
        3'b10: dout <= keys[23:16] ^ din;     // 3 keys
        3'b11: dout <= keys[31:24] ^ din;     // 4 keys
        default: dout <= 8'b0;
      endcase
      if(current_key == num_keys - 1) begin
        current_key <= 3'b0; // Reset to first key after using all keys
      end
      else begin
        current_key <= current_key + 1; // Move to the next key
      end

    end
  end
endmodule
      
  
  
