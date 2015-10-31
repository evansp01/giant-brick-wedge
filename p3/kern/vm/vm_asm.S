

.global invalidate_page
invalidate_page:
   movl 4(%esp), %ecx
   invlpg (%ecx)
   ret
