uint32_t parse_rr(uint32_t pos, uint32_t id_pos, struct pcap_pkthdr *header,
                 uint8_t *packet, dns_rr * rr, config * conf) {
   int i;
   uint32_t rr_start = pos;
   rr_parser_container * parser;
   rr_parser_container opts_cont = {0,0, opts};

   rr->name = NULL;
   rr->data = NULL;

   rr->name = read_rr_name(packet, &pos, id_pos, header->len);
   // Handle a bad rr name.
   // We still want to print the rest of the escaped rr data.
   if (rr->name == NULL) {
       const char * msg = "Bad rr name: ";
       rr->name = malloc(sizeof(char) * (strlen(msg) + 1));
       sprintf(rr->name, "%s", "Bad rr name");
       rr->type = 0;
       rr->rr_name = NULL;
       rr->cls = 0;
       rr->ttl = 0;
       rr->data = escape_data(packet, pos, header->len);
       return 0;
   }

   if ((header->len - pos) < 10 ) return 0;

   rr->type = (packet[pos] << 8) + packet[pos+1];
   rr->rdlength = (packet[pos+8] << 8) + packet[pos + 9];
   // Handle edns opt RR's differently.
   if (rr->type == 41) {
       rr->cls = 0;
       rr->ttl = 0;
       rr->rr_name = "OPTS";
       parser = &opts_cont;
       // We'll leave the parsing of the special EDNS opt fields to
       // our opt rdata parser.
       pos = pos + 2;
   } else {
       // The normal case.
       rr->cls = (packet[pos+2] << 8) + packet[pos+3];
       rr->ttl = 0;
       for (i=0; i<4; i++)
           rr->ttl = (rr->ttl << 8) + packet[pos+4+i];
       // Retrieve the correct parser function.
       parser = find_parser(rr->cls, rr->type);
       rr->rr_name = parser->name;
       pos = pos + 10;
   }

   VERBOSE(printf("Applying RR parser: %s\n", parser->name);)

   if (conf->MISSING_TYPE_WARNINGS && &default_rr_parser == parser)
       fprintf(stderr, "Missing parser for class %d, type %d\n",
                       rr->cls, rr->type);

   // Make sure the data for the record is actually there.
   // If not, escape and print the raw data.
   if (header->len < (rr_start + 10 + rr->rdlength)) {
       char * buffer;
       const char * msg = "Truncated rr: ";
       rr->data = escape_data(packet, rr_start, header->len);
       buffer = malloc(sizeof(char) * (strlen(rr->data) + strlen(msg) + 1));
       sprintf(buffer, "%s%s", msg, rr->data);
       free(rr->data);
       rr->data = buffer;
       return 0;
   }
   // Parse the resource record data.
   rr->data = parser->parser(packet, pos, id_pos, rr->rdlength,
                             header->len);
   VERBOSE(
   printf("rr->name: %s\n", rr->name);
   printf("type %d, cls %d, ttl %d, len %d\n", rr->type, rr->cls, rr->ttl,
          rr->rdlength);
   printf("rr->data %s\n", rr->data);
   )

   return pos + rr->rdlength;
}

char * read_rr_name(const uint8_t * packet, uint32_t * packet_p,
                   uint32_t id_pos, uint32_t len) {
   uint32_t i, next, pos=*packet_p;
   uint32_t end_pos = 0;
   uint32_t name_len=0;
   uint32_t steps = 0;
   char * name;

   // Scan through the name, one character at a time. We need to look at
   // each character to look for values we can't print in order to allocate
   // extra space for escaping them.  'next' is the next position to look
   // for a compression jump or name end.
   // It's possible that there are endless loops in the name. Our protection
   // against this is to make sure we don't read more bytes in this process
   // than twice the length of the data.  Names that take that many steps to
   // read in should be impossible.
   next = pos;
   while (pos < len && !(next == pos && packet[pos] == 0)
          && steps < len*2) {
       uint8_t c = packet[pos];
       steps++;
       if (next == pos) {
           // Handle message compression.
           // If the length byte starts with the bits 11, then the rest of
           // this byte and the next form the offset from the dns proto start
           // to the start of the remainder of the name.
           if ((c & 0xc0) == 0xc0) {
               if (pos + 1 >= len) return 0;
               if (end_pos == 0) end_pos = pos + 1;
               pos = id_pos + ((c & 0x3f) << 8) + packet[pos+1];
               next = pos;
           } else {
               name_len++;
               pos++;
               next = next + c + 1;
           }
       } else {
           if (c >= '!' && c <= 'z' && c != '\\') name_len++;
           else name_len += 4;
           pos++;
       }
   }
   if (end_pos == 0) end_pos = pos;

   // Due to the nature of DNS name compression, it's possible to get a
   // name that is infinitely long. Return an error in that case.
   // We use the len of the packet as the limit, because it shouldn't
   // be possible for the name to be that long.
   if (steps >= 2*len || pos >= len) return NULL;

   name_len++;

   name = (char *)malloc(sizeof(char) * name_len);
   pos = *packet_p;

   //Now actually assemble the name.
   //We've already made sure that we don't exceed the packet length, so
   // we don't need to make those checks anymore.
   // Non-printable and whitespace characters are replaced with a question
   // mark. They shouldn't be allowed under any circumstances anyway.
   // Other non-allowed characters are kept as is, as they appear sometimes
   // regardless.
   // This shouldn't interfere with IDNA (international
   // domain names), as those are ascii encoded.
   next = pos;
   i = 0;
   while (next != pos || packet[pos] != 0) {
       if (pos == next) {
           if ((packet[pos] & 0xc0) == 0xc0) {
               pos = id_pos + ((packet[pos] & 0x3f) << 8) + packet[pos+1];
               next = pos;
           } else {
               // Add a period except for the first time.
               if (i != 0) name[i++] = '.';
               next = pos + packet[pos] + 1;
               pos++;
           }
       } else {
           uint8_t c = packet[pos];
           if (c >= '!' && c <= '~' && c != '\\') {
               name[i] = packet[pos];
               i++; pos++;
           } else {
               name[i] = '\\';
               name[i+1] = 'x';
               name[i+2] = c/16 + 0x30;
               name[i+3] = c%16 + 0x30;
               if (name[i+2] > 0x39) name[i+2] += 0x27;
               if (name[i+3] > 0x39) name[i+3] += 0x27;
               i+=4;
               pos++;
           }
       }
   }
   name[i] = 0;

   *packet_p = end_pos + 1;

   return name;
}