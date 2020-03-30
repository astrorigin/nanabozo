<?
/**
 *  Very simple example with no main function.
 *  Compile with:
 *  nanabozo --main --html basic.php
 */

#define PAGE_TITLE "Hello World Example"

char *world = "World";
?>
<html>
  <head>
    <title><?= PAGE_TITLE ?></title>
  </head>
  <body>
    <h1><?% "Hello %s!", world ?></h1>
<?
for (int i = 0; i < 10; ++i) {
?>
      <p>Counting <?% "%d", i ?></p>
<?
} // end for
?>
  </body>
</html>
