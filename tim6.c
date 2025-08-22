void Sensor_TIM6_IRQ() {
   static int i = 0;
   GPIOC->ODR = ((GPIOC->ODR & ~0x0F) | (i + 8));
   for (int j = 0; j < 3; j++) {
      sensor_temp[j] = Sensor_ADC_Read() >> 4;
   }
   raw_sensor_array[7-i] = median3(sensor_temp[0], sensor_temp[1],sensor_temp[2]);
   GPIOC->ODR = ((GPIOC->ODR & ~0x0F) | i);

   if (calibrated) {
      static int prev_pos = 0;
      static int pos_target=0;
      //리미티트 포지션을 통한 포지션의 과도한 변경 방지
      if (sensor_range[i] == 0) {
         sensor_normalised[i] = 0;
      }
      else{int value = (255 * (raw_sensor_array[i] - sensor_max_black_array[i]))
            / sensor_range[i];
      sensor_normalised[i] = (value < 0) ? 0 : ((value > 255) ? 255 : value);
      }
      if (sensor_normalised[i] > threshold)
         sensor_binary_array[i] = 1;
      else
         sensor_binary_array[i] = 0;

      if (i == 7) {
         sensor_state = pack_sensor_binary();
         int32_t num = 0;
         int32_t sum = 0;
         int      start;

         // 만약 4개 이상의 센서가 라인을 감지했다면 교차로로 판단
         // 이 값(4)은 실험을 통해 트랙에 맞게 조정해야 합니다.
          // ── (A) 창 선택 ─────────────────────────
         if(window_able){
        	 if((v_target > curve_speed)){
        	            start = 3;
        	          for (int k = start; k <= start+3; k++) {
        	            num += (14000-4000*k) * sensor_normalised[k];
        	            sum += sensor_normalised[k];
        	          }
        	         }
        	         if (prev_pos >  6000.0f) {
        	            start = 1;
        	            for (int k = start; k <= start+2; k++) {
        	               num += (14000-4000*k) * sensor_normalised[k];
        	               sum += sensor_normalised[k];
        	            }
        	          }
        	          else if (prev_pos >  2000.0f) {
        	            start = 1;
        	            for (int k = start; k <= start+3; k++) {
        	               num += (14000-4000*k) * sensor_normalised[k];
        	               sum += sensor_normalised[k];
        	            }
        	          }
        	          else if (prev_pos >  -2000.0f) {
        	            start = 2;
        	            for (int k = start; k <= start+3; k++) {
        	               num += (14000-4000*k) * sensor_normalised[k];
        	               sum += sensor_normalised[k];
        	            }
        	          }
        	          else if (prev_pos >  -6000.0f) {
        	            start = 3;
        	            for (int k = start; k <= start+3; k++) {
        	              num += (14000-4000*k) * sensor_normalised[k];
        	              sum += sensor_normalised[k];
        	            }
        	          }
        	          else {
        	            start = 4;
        	            for (int k = start; k <= start+2; k++) {
        	               num += (14000-4000*k) * sensor_normalised[k];
        	               sum += sensor_normalised[k];
        	            }
        	          }
         }
         else{
        	 for (int k = 1; k <= 6; k++) {
			   num += (14000-4000*k) * sensor_normalised[k];
			   sum += sensor_normalised[k];
			}
         }

          if (sum == 0) {
            // Avoid division by zero; maintain last position
            line_pos = prev_pos;
          } else {
            // Correctly calculate the weighted average
            line_pos = (int) num / sum;
          }
          //IIR 재구현

          if(abs(line_pos)<=death_zone){
        	  line_pos = 0;
          }
          pos_target = line_pos;
          prev_pos = line_pos;
          pos = line_pos;
         }
      }
   i = (i + 1) & 7;
}