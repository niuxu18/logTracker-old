static int mec_destroy (void **user_data) /* {{{ */
{
  if (user_data != NULL)
  {
    sfree (*user_data);
  }

  return (0);
}