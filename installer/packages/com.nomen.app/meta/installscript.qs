function Component() {}

Component.prototype.createOperations = function()
{
    component.createOperations();
    
    // Linux:
    if (systemInfo.productType === "linux") 
    {
        component.addOperation("Execute", "{0,3010}", "chmod", "+x", "@TargetDir@/bin/Nomen");
    }
    
    // Windows
    // if (systemInfo.productType === "windows") {
    // }
};