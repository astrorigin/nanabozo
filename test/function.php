<?
/**
 *  Example with a main function.
 *  Compile with:
 *  nanabozo function.php
 */

#define PAGE_TITLE "Hello World Example"

char *world = "World";

int main(void) {
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
<?
    return 0;
} // end main

/**
 *  Beware of invisible chars stuck at end of file.
 */
?>



