#include <windows.h>
#include <ntstatus.h>
#include <stdio.h>
#include <ntsecapi.h>
#include <wchar.h>

int main() {
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE PolicyHandle;
    NTSTATUS Status;

    // Initialize the object attributes to zero
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

    // Open a policy handle with all access rights
    Status = LsaOpenPolicy(NULL, &ObjectAttributes, POLICY_ALL_ACCESS, &PolicyHandle);
    if (Status != STATUS_SUCCESS) {
        printf("Error opening policy handle: 0x%08X\n", Status);
        return 1;
    }

    // Initialize KeyName for SAM
    LSA_UNICODE_STRING KeyName;
    WCHAR Key[] = L"SAM"; // Key name to retrieve

    KeyName.Buffer = Key;
    KeyName.Length = (USHORT)(wcslen(Key) * sizeof(WCHAR)); // Length in bytes
    KeyName.MaximumLength = (USHORT)(KeyName.Length + sizeof(WCHAR)); // Include null terminator space

    // Retrieve private data
    LSA_UNICODE_STRING* PrivateData = NULL;
    Status = LsaRetrievePrivateData(PolicyHandle, &KeyName, &PrivateData);
    if (Status != STATUS_SUCCESS) {
        printf("Error retrieving SAM data: 0x%08X\n", Status);
        LsaClose(PolicyHandle);
        return 1;
    }

    // Print the retrieved data
    if (PrivateData && PrivateData->Buffer) {
        wprintf(L"SAM Data: %.*s\n", PrivateData->Length / sizeof(WCHAR), PrivateData->Buffer);
    } else {
        printf("No data retrieved.\n");
    }

    // Free allocated memory for PrivateData
    if (PrivateData) {
        LsaFreeMemory(PrivateData->Buffer);
        LsaFreeMemory(PrivateData);
    }

    // Close the policy handle
    LsaClose(PolicyHandle);

    return 0;
}
