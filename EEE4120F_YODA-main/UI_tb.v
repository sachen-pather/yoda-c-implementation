module UI_tb;
    // Registers to hold the key and message retrieved from the file
    integer file;
    integer i;

    reg [7:0] key_length;
    reg [7:0] c;
    reg [7:0] key [4:0];

    reg [7:0] message [1023:0];
    reg [10:0] message_length;

    reg [7:0] encrypted_message [1023:0];

    // Signal to go to actual DEA module
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

    initial begin

        


        file = $fopen("data.txt", "r");
        if (file == 0) begin
          $display("Failed to open file");
          $finish;
        end 
        key_length = 0;
        c = $fgetc(file);
        while (c != -1 && c != 8'h0A) begin  // -1 means EOF
          if (c != 8'h0A) begin
            key[key_length] = c[7:0];
            key_length = key_length + 1;
            c = $fgetc(file);// store ASCII char
          end  
        end 
        message_length = 0;
        c = $fgetc(file);
        while (c != -1 && c != 8'h0A) begin  // -1 means EOF
            if (c != 8'h0A) begin
            message[message_length] = c[7:0];
            message_length = message_length + 1;
            c = $fgetc(file);// store ASCII char
            end  
        end

        $fclose(file);

        dclk = 0;#10

        kset = 0;
        din = 8'h00;
        reset = 1;
        
        dclk = 1;#5;
        dclk = 0;#5;

        reset = 0;
        kset = 1;
        for (i = 0; i < key_length; i = i + 1) begin
            din = key[i];
            dclk = 1;#5;
            dclk = 0;#5;
        end

        // Switch to encryption mode (kset=0)
        kset = 0;
        for (i = 0; i < message_length; i = i + 1) begin
            din = message[i];
            dclk = 1;#5;
            dclk = 0;#5
            encrypted_message[i] = dout;
            //$display("Input: %h, Encrypted: %h (Expected: %h)", din, dout, key[i % (key_length)] ^ din);
        end

        // Display the encrypted message
        file = $fopen("output.txt", "w"); // "w" = write mode, "a" = append mode

        if (file == 0) begin
          $display("Failed to open file");
          $finish;
        end

        for (i = 0; i < message_length; i = i + 1) begin
            $fwrite(file, "%c",encrypted_message[i]);
            
        end
        $fclose(file);
      $finish;
    end

    //initial begin
    //$monitor("Time: %t | kset=%b num_keys=%d keys=%h din=%h dout=%h current_key=%h reset=%h dclk=%h",
    //         $time, kset, dut.k.num_keys, dut.k.keys, din, dout, dut.current_key, reset, dclk);
    //end


endmodule
