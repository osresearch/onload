#include "uapi_private.h"
#include <cplane/create.h>
#include <errno.h>
#include <ci/compat.h>
#include <onload/driveraccess.h>

int (* ci_sys_ioctl)(int, long unsigned int, ...) = ioctl;

EF_CP_PUBLIC_API
int ef_cp_init(struct ef_cp_handle **cph, unsigned flags)
{
  int rc = 0;
  struct ef_cp_handle *cp;

  if( flags )
    return -EINVAL;
  cp = calloc(1, sizeof(*cp));
  if( ! cp )
    return -ENOMEM;
  rc = oo_fd_open(&cp->drv_fd);
  if( rc )
    goto fail1;
  rc = oo_cp_create(cp->drv_fd, &cp->cp, CP_SYNC_LIGHT, 0);
  if( rc )
    goto fail2;
  cp->llap_extra = calloc(cp->cp.mib->dim->llap_max, sizeof(*cp->llap_extra));
  if( ! cp->llap_extra ) {
    rc = -ENOMEM;
    goto fail3;
  }
  *cph = cp;
  return 0;

fail3:
  oo_cp_destroy(&cp->cp);
fail2:
  oo_fd_close(cp->drv_fd);
fail1:
  free(cp);
  return rc;
}

EF_CP_PUBLIC_API
void ef_cp_fini(struct ef_cp_handle *cp)
{
  oo_cp_destroy(&cp->cp);
  oo_fd_close(cp->drv_fd);
  free(cp);
}
