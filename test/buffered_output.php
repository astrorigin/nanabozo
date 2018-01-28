<?
/*
 *  nanabozo -p 'ms.input' -f 'ms.inputf' ...
 */

#include <iostream>
#include "MemStream.cxx"

#define PAGE_TITLE "Whatever"

bool _headers_sent = false;

int main()
{
  MemStream ms;

  ms.input("Content-Type: text/html; charset=utf-8\n\n");
  cout << ms.toString();
  _headers_sent = true;
  ms.reset();
?>
<html>
 <title><?= PAGE_TITLE ?></title>
 <body>
<?
for (int i = 0; i < 10; ++i) {
?>
  <p><?% "Count: %d", i ?></p>
<?
}
?>
 </body>
</html>
<?
  cout << ms.toString();
  return 0;
} // end main()
?>
