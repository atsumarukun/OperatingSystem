#include  <Uefi.h>
#include  <Library/UefiLib.h>

EFI_STATUS UefiMain(EFI_HANDLE IMAGE_HANDLE, EFI_SYSTEM_TABLE *SYSTEM_TABLE) {
  Print(L"Hello Operating System.\n");
  while (1);
  return 0;
}