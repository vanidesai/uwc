# Python Publisher 

This is a sample python publisher which is used to test the data persistant feature by publishing a dummy json payload on the msg bus which is then stored on influx db.

## Steps to Run 

1. Add the following relative path in [uwc/uwc_recipes/uwc-pipeline-basic-timeseries.yml](../uwc_recipes/uwc-pipeline-basic-timeseries.yml).

```yaml
- uwc/Sample_Publisher
```

2. Follow the [uwc_README](../README.md) to run uwc.
